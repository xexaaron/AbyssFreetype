#include <CmdLine/CmdLine.h>
#include <FT/abyft.h>

#ifndef ABYSS_FT_VER
#   define ABYSS_FT_VER "Unknown"
#endif

struct InFontCfg {
    std::string file      = "";
    std::string pt        = "12";
    std::string dpi       = "96,96";
    std::string range     = "32,128";
    bool        verbose   = false;
    std::string cache_dir = ".";
};


bool parse_font_cfg(const InFontCfg& in_cfg, aby::ft::FontCfg& out_cfg) {
    // Parse point size
    try {
        out_cfg.pt = std::stoul(in_cfg.pt);
    } catch (const std::exception& e) {
        FT_ERROR("Failed to parse 'pt': ({}). {}.", in_cfg.pt, e.what());
        return false;
    }

    // Parse DPI
    try {
        std::stringstream ss(in_cfg.dpi);
        std::string token;
        float vals[2] = {96.f, 96.f};
        int i = 0;
        while (std::getline(ss, token, ',') && i < 2) {
            vals[i++] = std::stof(token);
        }
        if (i != 2) {
            FT_ERROR("DPI must contain two comma-seperated floats (e.g. \"96, 96\"): Got: ({}).", in_cfg.dpi);
            return false;
        }
        out_cfg.dpi = {vals[0], vals[1]};
    } catch (const std::exception& e) {
        FT_ERROR("Failed to parse 'dpi'. ({}). {}.", in_cfg.dpi, e.what());
        return false;
    }

    // Parse character range
    try {
        std::stringstream ss(in_cfg.range);
        std::string token;
        uint32_t vals[2] = {32, 128};
        int i = 0;
        while (std::getline(ss, token, ',') && i < 2) {
            vals[i++] = static_cast<uint32_t>(std::stoul(token));
        }
        if (i != 2) {
            FT_ERROR("Character range must contain two comma-separated integers (e.g. \"32,128\"). Got: ({}).", in_cfg.range);
            return false;
        }
        out_cfg.range = {static_cast<char32_t>(vals[0]), static_cast<char32_t>(vals[1])};
    } catch (const std::exception& e) {
        FT_ERROR("Failed to parse 'range'. ({}). {}.", in_cfg.range, e.what());
        return false;
    }

    out_cfg.verbose = in_cfg.verbose;
    out_cfg.path = in_cfg.file;
    
    return true;
}

int main(int argc, char** argv) {
    aby::util::CmdLine cmd;
    aby::util::CmdLine::Opts opts{
        .desc = "Cmdline utility for use of AbyssFreetype library",     
        .name = "AbyssFreetype",
        .cerr = std::cerr,         
        .help = true,        
        .term_colors = true,  
    };

    InFontCfg in_cfg;
    aby::ft::FontCfg out_cfg;
    bool version;

    if (!cmd.opt("file", "Font file to load", &in_cfg.file, true)
       .opt("pt", "Requested point size of font (Default: '12')", &in_cfg.pt)
       .opt("dpi", "Dots per inch (Default: '96,96')", &in_cfg.dpi)
       .opt("range", "Character range to load (Default: '32,128')", &in_cfg.range)
       .opt("cache_dir", "Directory to output cached png and binary glyph to (Default '.')", &in_cfg.cache_dir)
       .flag("version", "Display version number and build info", &version, false, {"file"})
       .flag("v", "Enable verbose log messages", &in_cfg.verbose, false)
       .parse(argc, argv, opts) || !parse_font_cfg(in_cfg, out_cfg))
    {
        return 1;
    }

    if (version) {
#ifndef NDEBUG
        std::string build_mode = "Debug";
#else
        std::string build_mode = "Release";
#endif
        FT_STATUS("Version: {} {}", ABYSS_FT_VER, build_mode);
        return 0;
    }

    aby::ft::Library& lib = aby::ft::Library::get();
    aby::ft::FontData data = lib.create_font_data(in_cfg.cache_dir, out_cfg);

    FT_STATUS("Succesfully Loaded font file: {}", in_cfg.file);
    FT_STATUS(" -- Name:        {}", data.name);
    FT_STATUS(" -- Glyphs:      {}", data.glyphs.size());
    FT_STATUS(" -- Monospaced:  {}", data.is_mono);
    FT_STATUS(" -- Text Height: {}", data.text_height);
    FT_STATUS(" -- Output PNG:  {}", std::filesystem::absolute(data.png).string());
    FT_STATUS(" -- Point Size:  {}", out_cfg.pt);
    FT_STATUS(" -- DPI:         ({}, {})", out_cfg.dpi.x, out_cfg.dpi.y);
    FT_STATUS(" -- Char Range:  ({}, {})", static_cast<uint32_t>(out_cfg.range.start), static_cast<uint32_t>(out_cfg.range.end));

    return 0;
}