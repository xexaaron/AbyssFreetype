#include <CmdLine/CmdLine.h>
#include <FT/abyft.h>
#include <PrettyPrint/PrettyPrint.h>

#ifndef ABYSS_FT_VER
#	define ABYSS_FT_VER "Unknown"
#endif

namespace aby::util {

	struct InFontCfg {
		std::string file      = "";
		std::string pt        = "12";
		std::string dpi       = "96,96";
		std::string range     = "32,128";
		bool verbose          = false;
		std::string cache_dir = ".";
	};

	bool parse_font_cfg(const InFontCfg& in_cfg, aby::ft::FontCfg& out_cfg) {
		// Parse point size
		std::string parse_errors;
		try {
			out_cfg.pt = std::stoul(in_cfg.pt);
		} catch (const std::exception& e) {
			parse_errors += std::format("  Failed to parse 'pt': ({}). {}.\n", in_cfg.pt, e.what());
		}

		// Parse DPI
		try {
			std::stringstream ss(in_cfg.dpi);
			std::string token;
			float vals[2] = { 96.f, 96.f };
			int i         = 0;
			while (std::getline(ss, token, ',') && i < 2) {
				vals[i++] = std::stof(token);
			}
			if (i != 2) {
				parse_errors += std::format("  DPI must contain two comma-seperated floats (e.g. \"96, 96\"): Got: ({}).\n", in_cfg.dpi);
			}
			out_cfg.dpi = { vals[0], vals[1] };
		} catch (const std::exception& e) {
			parse_errors += std::format("  Failed to parse 'dpi'. ({}). {}.\n", in_cfg.dpi, e.what());
		}

		// Parse character range
		try {
			std::stringstream ss(in_cfg.range);
			std::string token;
			uint32_t vals[2] = { 32, 128 };
			int i            = 0;
			while (std::getline(ss, token, ',') && i < 2) {
				vals[i++] = static_cast<uint32_t>(std::stoul(token));
			}
			if (i != 2) {
				parse_errors += std::format("  Character range must contain two comma-separated integers (e.g. \"32,128\"). Got: ({}).\n", in_cfg.range);
			}
			out_cfg.range = { static_cast<char32_t>(vals[0]), static_cast<char32_t>(vals[1]) };
		} catch (const std::exception& e) {
			parse_errors += std::format("  Failed to parse 'range'. ({}). {}.\n", in_cfg.range, e.what());
		}

		out_cfg.verbose = in_cfg.verbose;
		out_cfg.path    = in_cfg.file;

		if (!parse_errors.empty()) {
			pretty_print(parse_errors, "Errors", Colors{ .box = EColor::RED, .ctx = EColor::YELLOW });
			return false;
		}

		return true;
	}
} // namespace aby::util
int main(int argc, char** argv) {
	aby::util::CmdLine cmd;
	aby::util::CmdLine::Opts opts{
		.desc        = "Cmdline utility for use of AbyssFreetype library",
		.name        = "AbyssFreetype",
		.cerr        = std::cerr,
		.help        = true,
		.term_colors = true,
	};

	aby::util::InFontCfg in_cfg;
	aby::ft::FontCfg out_cfg;
	bool version = false;
	bool quiet   = false;

	if (!cmd.opt("file", "Font file to load", &in_cfg.file, true)
	         .opt("pt", "Requested point size of font (Default: '12')", &in_cfg.pt)
	         .opt("dpi", "Dots per inch (Default: '96,96')", &in_cfg.dpi)
	         .opt("range", "Character range to load (Default: '32,128')", &in_cfg.range)
	         .opt("cache_dir", "Directory to output cached png and binary glyph to (Default '.')", &in_cfg.cache_dir)
	         .flag("version", "Display version number and build info", &version, false, { "file" })
	         .flag("v", "Enable verbose log messages", &in_cfg.verbose)
	         .flag("q", "Suppress output log messages", &quiet)
	         .parse(argc, argv, opts) ||
	    !parse_font_cfg(in_cfg, out_cfg))
	{
		return 1;
	}

	if (quiet) out_cfg.verbose = false;

	if (version) {
#ifndef NDEBUG
		std::string build_mode = "Debug";
#else
		std::string build_mode = "Release";
#endif
		aby::util::pretty_print(
		    std::format("  Version: \033[30m{}\033[0m {}               ", aby::ft::Library::version(), build_mode),
		    "AbyssFreetype",
		    aby::util::Colors{
		        .box = aby::util::EColor::GREEN,
		        .ctx = aby::util::EColor::YELLOW,
		    });
		return 0;
	}

	aby::ft::Library& lib  = aby::ft::Library::get();
	aby::ft::FontData data = lib.create_font_data(in_cfg.cache_dir, out_cfg);

	if (!quiet) {
		std::string load_info;
		load_info += std::format("  Succesfully Loaded font file: \033[4m\033[34m{}\033[0m\n", in_cfg.file);
		load_info += std::format("    \033[36mName:        \033[0m\033[30m{}\033[0m\n", data.name);
		load_info += std::format("    \033[36mGlyphs:      \033[0m\033[30m{}\033[0m\n", data.glyphs.size());
		load_info += std::format("    \033[36mMonospaced:  \033[0m\033[30m{}\033[0m\n", data.is_mono);
		load_info += std::format("    \033[36mText Height: \033[0m\033[30m{}\033[0m\n", data.text_height);
		load_info += std::format("    \033[36mOutput PNG:  \033[0m\033[4m\033[34m{}\033[0m\n", std::filesystem::absolute(data.png).string());
		load_info += std::format("    \033[36mPoint Size:  \033[0m\033[30m{}\033[0m\n", out_cfg.pt);
		load_info += std::format("    \033[36mDPI:         \033[0m\033[30m({}, {})\033[0m\n", out_cfg.dpi.x, out_cfg.dpi.y);
		load_info += std::format("    \033[36mChar Range:  \033[0m\033[30m({}, {})\033[0m\n", static_cast<uint32_t>(out_cfg.range.start), static_cast<uint32_t>(out_cfg.range.end));

		aby::util::pretty_print(load_info, "AbyssFreetype", aby::util::Colors{ .box = aby::util::EColor::GREEN, .ctx = aby::util::EColor::YELLOW });
	}
	return 0;
}
