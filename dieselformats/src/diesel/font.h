#pragma once

///
/// Functions and classes to read from Diesel's various font files (.font, .blb and .abc). Writing is not supported and reading from .abc (FontMakerFont) files does nothing.
///

#include "fileio/reader.h"

#include "diesel/shared.h"
#include "diesel/modern/modern_shared.h"

#include <map>
#include <string>
#include <vector>

namespace diesel {
  class Glyph {
  public:
    uint8_t texture_index;
    uint8_t w;
    uint8_t h;
    int8_t xadvance;
    int8_t xoffset;
    int8_t yoffset;
    uint16_t x;
    uint16_t y;
  };


  class AngelCodeFont { // technically AngelCodeFont::Data, as no texture data is processed here
  public:
    // Reader must belong to a .font or .blb font file. The provided engine version will be used to determine which version to load.
    bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);
  private:
    // Loads .blb font files
    void load_from_legacy(Reader& reader, const DieselFormatsLoadingParameters& version);
    // Loads .font files
    void load_from_modern(Reader& reader, const DieselFormatsLoadingParameters& version);

  public:
    static std::string DumpFontToXml(const AngelCodeFont& font);
  private:
    std::vector<Glyph> _glyphs;
    std::map<int, int> _character_to_glyph_map;
    std::map<std::pair<int, int>, int> _kerning;
    std::string _face;

    int _size;
    int _texture_width;
    int _texture_height;
    int _base_line;
    int _line_height;
  };


  struct GLYPH_ATTR_4 {
    float left;
    float top;
    float right;
    float bottom;
    int16_t wOffset;
    int16_t wWidth;
    int16_t wAdvance;
  };
  struct FILE_GLYPH_ATTR {
    float fLeft;
    float fTop;
    float fRight;
    float fBottom;
    int16_t wOffset;
    int16_t wWidth;
    int16_t wAdvance;
    int16_t wPad;
  };

  class FontMakerFont {
  public:
    // Reader must belong to a .abc file.
    FontMakerFont(Reader& reader, const DieselFormatsLoadingParameters& version);

  private:
    int _texture_height;
    int _texture_width;

    int _size;
    int _line_height;

    std::vector<std::pair<int, int>> _letters;
    std::vector<diesel::Glyph> _glyphs;
  };
}