#include <filesystem>
#include <optional>
#include <PrettyPrint/PrettyPrint.h>
#include "FT/abyft.h"

#ifdef _WIN32
#	include <windows.h>
#endif

namespace aby::ft::test {

	std::filesystem::path CACHE_DIR = "./Cache";

	std::optional<FontData> load_ttf(const std::filesystem::path& font) {
		std::filesystem::path FONT = font;

#ifdef _WIN32
		UINT dpi = GetDpiForSystem();
#else
		float dpi = 96;
#endif

		FontCfg cfg{
			.pt    = 14,
			.dpi   = { static_cast<float>(dpi), static_cast<float>(dpi) },
			.range = {                      32,                     128 },
			.path  = FONT,
		};

		std::optional<FontData> data = Library::get().create_font_data(CACHE_DIR, cfg);

		if (!data) {
			FT_ERROR("Font: {} failed to load font data", FONT.string());
			return std::nullopt;
		}

		if (!data.value().is_mono) {
			FT_ERROR("Font: {} is supposed to be monospaced", FONT.string());
		}

		if (data.value().name != cfg.path.filename().string()) {
			FT_ERROR("Font: {} is supposed to have name of {}", FONT.string(), cfg.path.filename().string());
		}

		if (!std::filesystem::exists(data.value().png)) {
			FT_ERROR("Font: {} failed to output PNG file", FONT.string());
		}

		return data;
	}

} // namespace aby::ft::test

int main(int argc, char** argv) {
	int res = 0;

	std::filesystem::path font_dir = std::filesystem::path(argv[0]).parent_path() / "AbyssFreetypeTests" / "Fonts";

	{
		auto ttf = aby::ft::test::load_ttf(font_dir / "IBMPlexMono" / "IBMPlexMono-Regular.ttf");
		if (!ttf.has_value()) {
			FT_ERROR("Test Failed: {}", "Load TTF");
			res = 1;
		} else {
			FT_STATUS("Test Succeeded: {}", "Load TTF");
			aby::ft::FontData fd = ttf.value();
			std::string info, glyphs;
			info += "  Name:   " + fd.name + "\n";
			info += "  Mono:   " + std::to_string(fd.is_mono) + "\n";
			info += "  PNG:    " + fd.png.string() + "\n";
			info += "  Height: " + std::to_string(fd.text_height) + "\n";
			info += "  Glyphs: " + std::to_string(fd.glyphs.size())  + "\n";
			for (auto& [c, g] : fd.glyphs) {
				info += "    Char:    " + std::string(1, c) + (c == U' ' ? " " : "") + "\n";
				info += "    Advance: "  + std::to_string(g.advance) + "\n";
				info += "    Bearing: (" + std::to_string(g.bearing.x) + ", " + std::to_string(g.bearing.y) + ")\n";
				info += "    Offset:  " + std::to_string(g.offset) + "\n";
				info += "    Size:    (" + std::to_string(g.size.x) + ", " + std::to_string(g.size.y) + ")\n";
				info += "    Texcoords: 4\n"; 
				info += "       Top-Left:     (" + std::to_string(g.texcoords[0].x) + ", " + std::to_string(g.texcoords[0].y) + ")\n";   
				info += "       Top-Right:    (" + std::to_string(g.texcoords[1].x) + ", " + std::to_string(g.texcoords[1].y) + ")\n";
				info += "       Bottom-Right: (" + std::to_string(g.texcoords[2].x) + ", " + std::to_string(g.texcoords[2].y) + ")\n";
				info += "       Bottom-Left:  (" + std::to_string(g.texcoords[3].x) + ", " + std::to_string(g.texcoords[3].y) + ")\n";
			}
			aby::util::pretty_print(info, "AbyssFTTest");

		}
	}

	
	return res;
}
