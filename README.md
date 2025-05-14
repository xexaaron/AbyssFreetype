# README.md

## Description

AbyssFreetype is a C++ wrapper library over freetype and freetypes dependencies (brotli, bzip2, harfbuzz, libpng, zlib),
without relying on a package manager and or installed libraries using cmake.

It is used in [AbyssEngine](https://github.com/xexaaron/AbyssEngine) to load the fonts.

Additionally the project includes a cmdline utility encapsulating all functionality
of the library.

## Cloning / Building

### Cloning

```bash
git clone --branch master --recurse-submodules --shallow-submodules https://github.com/xexaaron/AbyssFreetype.git
```

### Building

```bash
cd AbyssFreetype
# Directory must be named build to work with certain development tools
mkdir build
cd build
cmake ..
# Default build mode is Debug
cmake --build .
```

## Example

```cpp
#include <FT/abyft.h>

int main(int argc, char** argv) {
    std::filesystem::path exec_dir  = std::filesystem::path(argv[0]).parent_path();
    std::filesystem::path font_dir  = exec_dir / "Fonts";
    std::filesystem::path font_path = font_dir / "IBMPlexMono" / "IBMPlexMono-Regular.ttf";
    std::filesystem::path cache_dir = exec_dir / "Cache";

    aby::ft::FontCfg cfg{
        .pt    = 14,          // Size in points
        .dpi   = { 96,  96 }, // Get system dpi or window dpi, use 96 as generic default.
        .range = { 32, 128 }, // Ascii character range
        .path  = font_path,   // Path to font file
    };

    // The Library class is a singleton and will be initialized the first time get is called
    // and deinitialized during static variable cleanup.
    aby::ft::Library font_lib = aby::ft::Library::get();

    // When calling 'create_font_data' it will first check if the cached files exist in
    // the passed in cache directory, if not then it will create them there.
    aby::ft::FontData font_data = font_lib.create_font_data(cache_dir, cfg);

    font_data.glyphs;      // Contains the glyphs in a map accessible by using char32_t as a key.
    font_data.name;        // filename (in this case IBMPlexMono-Regular.ttf).
    font_data.is_mono;     // Boolean indicating if the font is monospaced.
    font_data.png;         // Output png file of the font. Ready to be used in a texture.
    font_data.text_height; // Height of the font in pixels.
}
```

## Font Cache Format

Fonts get cached as an image file (.png) and a binary file containing information on the glyphs.

The character start and end range does not always equal the glyph count.
Therefore to parse it we can not use the range to make any guarantees about the size
of the data.

### Variables

```yaml
FontName:  The filename that was used to load the font.
FontExt:   The file extension of the loaded font.
Start:     The begin range of the font glyphs. (ie. ascii would be 32-128)
End:       The end range of the font glyphs.
Pt:        The pt size of the font when loaded. 
```

### Filepath naming

```yaml
Filepath: "[FontName].[FontExt]_[Start]_[End]_[Pt].bin"
```

### File Format

```yaml
GlyphCount:      8  byte uint
TextHeight:      4  byte float
Glyphs:          56 byte struct
    advance:     4  byte uint
    offset:      4  byte uint
    bearing:     8  byte fvec2
    size:        8  byte fvec2
    texcoords:   32 byte fvec2[4]
 ```
