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

	struct Glyph {
		u32 advance       = 0;
		u32 offset        = 0;
		vec2 bearing      = { 0.f, 0.f };
		vec2 size         = { 0.f, 0.f };
		vec2 texcoords[4] = {
			{ 0.f, 0.f },
            { 0.f, 0.f },
            { 0.f, 0.f },
            { 0.f, 0.f }
		};
		u64 reserved = 0;
	};
	static_assert(sizeof(Glyph) == 64);
	using Glyphs = std::unordered_map<char32_t, Glyph>;

	struct FontData {
		Glyphs glyphs             = {};
		float text_height         = 0.f;
		bool is_mono              = false;
		std::string name          = "";
		std::filesystem::path png = "";
	};

	struct CharRange {
		char32_t start = 32;
		char32_t end   = 128;
	};

	struct FontCfg {
		u32 pt                     = 14;
		vec2 dpi                   = { 96.f, 96.f };
		CharRange range            = { 32, 128 };
		std::filesystem::path path = "";
		bool verbose               = false;
	};

	struct Version {
		constexpr Version(std::uint32_t major, std::uint32_t minor, std::uint32_t patch) :
		    value((major & 0xFF) << 24 | (minor & 0xFF) << 16 | (patch & 0xFF) << 8) {
		}
		constexpr Version(std::uint32_t value) : value(value) {}

		constexpr std::tuple<std::uint32_t, std::uint32_t, std::uint32_t> get() const {
			return {
				(value >> 24) & 0xFF,
				(value >> 16) & 0xFF,
				(value >> 8) & 0xFF
			};
		}

		constexpr bool operator==(const Version& other) const {
			return value == other.value;
		}
		constexpr bool operator!=(const Version& other) const {
			return value != other.value;
		}
		
		std::uint32_t value;
	};

} // namespace aby::ft

namespace std {

	template <>
	class formatter<aby::ft::Version> {
	public:
		template <class ParseCtx>
		constexpr ParseCtx::iterator parse(ParseCtx& ctx) {
			return ctx.begin();
		}

		template <class FmtCtx>
		FmtCtx::iterator format(const aby::ft::Version& v, FmtCtx& ctx) const {
			auto [major, minor, patch] = v.get();
			return std::format_to(ctx.out(), "{}.{}.{}", major, minor, patch);
		}
	};

} // namespace std

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
		
		static constexpr Version version() { return s_Version; }
	private:
		::FT_FaceRec_* create_face(const FontCfg& cfg);
		void destroy_face(::FT_FaceRec_* face, const FontCfg& cfg);
		FontData load_glyph_range(const std::filesystem::path& cache_dir, const FontCfg& cfg);
		FontData load_glyph_range_ttf(FT_FaceRec_* face, const std::filesystem::path& png_file, const FontCfg& cfg);
		FontData load_glyph_range_bin(const std::filesystem::path& cache, const FontCfg& cfg);
		void cache_glyphs(const std::filesystem::path& cache_dir, const std::string& name, const FontData& data, const FontCfg& cfg);
		std::filesystem::path cache_path(const std::filesystem::path& cache_dir, const std::string& name, const std::filesystem::path& ext, const FontCfg& cfg);

		Library();
		~Library();
	private:
		::FT_LibraryRec_* m_Library               = nullptr;
		std::ostringstream m_VerboseStream		  = {};		
		static inline constexpr Version s_Version = Version(ABY_FT_VER_MAJOR, ABY_FT_VER_MINOR, ABY_FT_VER_PATCH);
	};

} // namespace aby::ft
