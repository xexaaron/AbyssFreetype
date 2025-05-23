#include "FT/abyft.h"
#include "FT/serializer.h"

#include <freetype/freetype.h>
#include <stb/stb_image_write.h>
#include <PrettyPrint/PrettyPrint.h>

#include <chrono>
#include <format>
#include <iostream>

namespace aby::ft {

	Library::Library() {
		FT_CHECK(::FT_Init_FreeType(&m_Library));
	}

	Library::~Library() {
		FT_CHECK(::FT_Done_FreeType(m_Library));
	}

	Library& Library::get() {
		static Library lib;
		return lib;
	}

	FontData Library::create_font_data(const std::filesystem::path& cache_dir, const FontCfg& cfg) {
		FontData data =  load_glyph_range(cache_dir, cfg);
		if (cfg.verbose) {
			m_VerboseStream.clear();
			util::pretty_print(m_VerboseStream.str(), "AbyssFreetype");
		}
		return data;
	}

	::FT_FaceRec_* Library::create_face(const FontCfg& cfg) {
		FT_Face face         = nullptr;
		std::string path_str = cfg.path.string();
		FT_CHECK(::FT_New_Face(m_Library, path_str.c_str(), FT_Long(0), &face));
		FT_CHECK(::FT_Set_Char_Size(face, FT_F26Dot6(0), cfg.pt << 6u, static_cast<FT_UInt>(cfg.dpi.x), static_cast<FT_UInt>(cfg.dpi.y)));
		return face;
	}

	void Library::destroy_face(::FT_FaceRec_* face, const FontCfg& cfg) {
		::FT_Done_Face(face);
	}

	FontData Library::load_glyph_range(const std::filesystem::path& cache_dir, const FontCfg& cfg) {
		auto name       = cfg.path.filename().string();
		auto glyph_file = cache_path(cache_dir, name, ".bin", cfg);
		auto png_file   = cache_path(cache_dir, name, ".png", cfg);
		FontData out;

		std::chrono::time_point<std::chrono::high_resolution_clock> start;
		if (cfg.verbose) {
			start = std::chrono::high_resolution_clock::now();
		}

		if (std::filesystem::exists(png_file) && std::filesystem::exists(glyph_file)) {
			if (cfg.verbose) {
				m_VerboseStream << std::format("  Loading font from cache: \x1b[4;34m{}\x1b[0m\n\n", glyph_file.string());
			}
			out = load_glyph_range_bin(glyph_file, cfg);
		} else {
			if (cfg.verbose) {
				m_VerboseStream << std::format("  Loading font from file: \x1b[4;34m{}\x1b[0m\n\n", cfg.path.string());
			}
			FT_Face face = create_face(cfg);
			out          = load_glyph_range_ttf(face, png_file, cfg);
			cache_glyphs(cache_dir, name, out, cfg);
			destroy_face(face, cfg);
		}

		if (cfg.verbose) {
			using clock = std::chrono::high_resolution_clock;
			using ns    = std::chrono::nanoseconds;
			float elapsed = std::chrono::duration_cast<ns>(clock::now() - start).count() * 0.001f * 0.001f;
			m_VerboseStream << std::format("  Font Loading took a total of \x1b[2;38;5;120m{}\x1b[0mms\n", elapsed);
		}

		out.name = name;
		out.png  = png_file;
		return out;
	}

	FontData Library::load_glyph_range_ttf(FT_FaceRec_* face, const std::filesystem::path& png_file, const FontCfg& cfg) {
		u32 tex_width  = 512;
		u32 tex_height = 512;

		std::vector<char> pixels(tex_width * tex_height, 0); // Initialize pixel buffer with 0 (black)

		int pen_x = 0;
		int pen_y = 0;

		float max_ascent  = static_cast<float>(face->size->metrics.ascender) / 64.0f;
		float max_descent = static_cast<float>(face->size->metrics.descender) / 64.0f;
		FontData out{
			.glyphs      = {},
			.text_height = (max_ascent - max_descent) * 0.5f,
			.is_mono     = static_cast<bool>(face->face_flags & FT_FACE_FLAG_FIXED_WIDTH),
		};

		for (char32_t c = cfg.range.start; c < cfg.range.end; ++c) {
			int pad = 1;
			if (::FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT))
				continue; // If it is not a valid character, continue

			auto* glyph = face->glyph;
			auto* bmp   = &glyph->bitmap;

			// If the glyph doesn't fit in the current row, move to the next row
			if (pen_x + bmp->width >= tex_width) {
				pen_x  = 0;
				pen_y += (face->size->metrics.height >> 6) + pad;
			}
			vec2 uv_min   = { static_cast<float>(pen_x) / tex_width, static_cast<float>(pen_y) / tex_height };
			vec2 uv_max   = { static_cast<float>(pen_x + bmp->width) / tex_width, static_cast<float>(pen_y + bmp->rows) / tex_height };
			vec4 uvs      = { uv_min.x, uv_min.y, uv_max.x, uv_max.y };
			out.glyphs[c] = Glyph{
				.advance   = static_cast<u32>(glyph->advance.x >> 6u),
				.offset    = pen_y * tex_width + pen_x,
				.bearing   = { static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top) },
				.size      = { static_cast<float>(bmp->width), static_cast<float>(bmp->rows) },
				.texcoords = {
					{ uvs.x, uvs.y }, // Top-left  (0)
				    { uvs.z, uvs.y }, // Top-right (1)
				    { uvs.z, uvs.w }, // Bottom-right (2)
				    { uvs.x, uvs.w }  // Bottom-left  (3)
				},
			};

			for (unsigned int row = 0; row < bmp->rows; ++row) {
				for (unsigned int col = 0; col < bmp->width; ++col) {
					unsigned int x            = pen_x + col;
					unsigned int y            = pen_y + row;
					pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col]; // Copy pixel data
				}
			}

			pen_x += bmp->width + pad;
		}

		std::vector<unsigned char> png_data(tex_width * tex_height * 4);
		for (unsigned int i = 0; i < tex_width * tex_height; ++i) {
			png_data[i * 4 + 0] = pixels[i]; // Red channel
			png_data[i * 4 + 1] = pixels[i]; // Green channel
			png_data[i * 4 + 2] = pixels[i]; // Blue channel
			png_data[i * 4 + 3] = 0xff;      // Alpha channel (fully opaque)
		}

		std::string png_file_str = png_file.string();
		::stbi_write_png(png_file_str.c_str(), tex_width, tex_height, 4, png_data.data(), tex_width * 4);

		return out;
	}

	FontData Library::load_glyph_range_bin(const std::filesystem::path& cache, const FontCfg& cfg) {
		Serializer serializer(SerializeOpts{ .file = cache, .mode = ESerializeMode::READ });
		std::uint32_t version = 0;
		std::size_t glyph_count = 0;
		FontData out;

		serializer.read(version);

		FT_ASSERT(Version(version) == s_Version, 
			"  Version of cached font glyphs does not match library version\n"
			"  File Version:    {}\n"
			"  Library Version: {}", 
			Version(version), s_Version
		);

		serializer.read(glyph_count);
		serializer.read(out.text_height);
		serializer.read(out.is_mono);

		out.glyphs.reserve(glyph_count);
		for (std::size_t i = 0; i < glyph_count; i++) {
			char32_t character;
			Glyph glyph;
			serializer.read(character);
			serializer.read(glyph.advance);
			serializer.read(glyph.offset);
			serializer.read(glyph.bearing.x);
			serializer.read(glyph.bearing.y);
			serializer.read(glyph.size.x);
			serializer.read(glyph.size.y);
			serializer.read(glyph.texcoords[0].x);
			serializer.read(glyph.texcoords[0].y);
			serializer.read(glyph.texcoords[1].x);
			serializer.read(glyph.texcoords[1].y);
			serializer.read(glyph.texcoords[2].x);
			serializer.read(glyph.texcoords[2].y);
			serializer.read(glyph.texcoords[3].x);
			serializer.read(glyph.texcoords[3].y);
			serializer.read(glyph.reserved);
			out.glyphs.emplace(character, glyph);
		}

		return out;
	}

	void Library::cache_glyphs(const std::filesystem::path& cache_dir, const std::string& name, const FontData& data, const FontCfg& cfg) {
		std::filesystem::path bin_cache_path = cache_path(cache_dir, name, ".bin", cfg);

		Serializer serializer(SerializeOpts{ .file = bin_cache_path, .mode = ESerializeMode::WRITE });
		serializer.write(s_Version.value);
		serializer.write(data.glyphs.size());
		serializer.write(data.text_height);
		serializer.write(data.is_mono);
		for (const auto& [character, glyph] : data.glyphs) {
			serializer.write(character);
			serializer.write(glyph.advance);
			serializer.write(glyph.offset);
			serializer.write(glyph.bearing.x);
			serializer.write(glyph.bearing.y);
			serializer.write(glyph.size.x);
			serializer.write(glyph.size.y);
			serializer.write(glyph.texcoords[0].x);
			serializer.write(glyph.texcoords[0].y);
			serializer.write(glyph.texcoords[1].x);
			serializer.write(glyph.texcoords[1].y);
			serializer.write(glyph.texcoords[2].x);
			serializer.write(glyph.texcoords[2].y);
			serializer.write(glyph.texcoords[3].x);
			serializer.write(glyph.texcoords[3].y);
			serializer.write(glyph.reserved);
		}
		serializer.save();
	}

	std::filesystem::path Library::cache_path(const std::filesystem::path& cache_dir, const std::string& name, const std::filesystem::path& ext, const FontCfg& cfg) {
		auto dir = cache_dir / "Fonts";
		if (!std::filesystem::exists(dir)) {
			std::filesystem::create_directories(dir);
		}
		auto path = name + "_" + std::to_string(cfg.range.start) + "_" + std::to_string(cfg.range.end) + "_" + std::to_string(cfg.pt) + ext.string();

		return dir / path;
	}

} // namespace aby::ft
