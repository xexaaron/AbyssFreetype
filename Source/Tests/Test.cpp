#include <filesystem>
#include <optional>

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

	std::filesystem::path font_dir = std::filesystem::path(argv[0]).parent_path() / "Fonts";

	{
		auto ttf = aby::ft::test::load_ttf(font_dir / "IBMPlexMono" / "IBMPlexMono-Regular.ttf");
		if (!ttf.has_value()) {
			FT_ERROR("Test Failed: {}", "Load TTF");
			res = 1;
		} else {
			FT_STATUS("Test Succeeded: {}", "Load TTF");
		}
	}

	
	return res;
}
