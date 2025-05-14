#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "FT/common.h"

namespace aby::ft {

	struct vec2 {
		float x = 0.f;
		float y = 0.f;
	};

	struct vec4 {
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
		float w = 0.f;
	};

	// Glyph Cache Format
	// File: ${m_Name}_${character_start_range}_${character_end_range}_${m_SizePt}.bin
	// Layout:
	//      std::size_t glyph_count. (Not always character_end_range - character_start_range)
	//      float       text_height.
	//      bool        is_monospaced.
	//      {
	//          char32_t    character
	//          Glyph       glyph
	//      }...
	//
	//
	struct Glyph {
		u32 advance;
		u32 offset;
		vec2 bearing;
		vec2 size;
		vec2 texcoords[4];
	};

	using Glyphs = std::unordered_map<char32_t, Glyph>;

	struct FontData {
		Glyphs glyphs             = {};
		float text_height         = 0.f;
		bool is_mono              = false;
		std::string name          = "";
		std::filesystem::path png = "";
	};

	struct CharRange {
		char32_t start;
		char32_t end;
	};

	struct FontCfg {
		u32 pt;
		vec2 dpi;
		CharRange range;
		std::filesystem::path path;
	};

} // namespace aby::ft

struct FT_FaceRec_;
struct FT_LibraryRec_;

namespace aby::ft {

	/**
     * @brief Font Library Singleton class
    */
	class Library {
	public:
		static Library& get();

		FontData create_font_data(const std::filesystem::path& cache_dir, const FontCfg& cfg);
	private:
		::FT_FaceRec_* create_face(const FontCfg& cfg);
		void destroy_face(::FT_FaceRec_* face);
		FontData load_glyph_range(const std::filesystem::path& cache_dir, const FontCfg& cfg);
		FontData load_glyph_range_ttf(FT_FaceRec_* face, const std::filesystem::path& png_file, const FontCfg& cfg);
		FontData load_glyph_range_bin(const std::filesystem::path& cache, const FontCfg& cfg);
		void cache_glyphs(const std::filesystem::path& cache_dir, const std::string& name, const FontData& data, const FontCfg& cfg);
		std::filesystem::path cache_path(const std::filesystem::path& cache_dir, const std::string& name, const std::filesystem::path& ext, const FontCfg& cfg);

		Library();
		~Library();
	private:
		::FT_LibraryRec_* m_Library = nullptr;
	};

} // namespace aby::ft
