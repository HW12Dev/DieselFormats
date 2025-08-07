#include "diesel/font.h"

#include "diesel/modern/modern_shared.h"

#include "fileio/reader.h"

#include <exception>

static_assert(sizeof(diesel::Glyph) == 10);

namespace diesel {
  bool AngelCodeFont::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
    if (!VerifyBlobType(reader, 0x3730537A))
      return false;

    if (version.version < diesel::EngineVersion::MODERN_VERSION_START) {
      this->load_from_legacy(reader, version);
      return true;
    }
    else {
      this->load_from_modern(reader, version);
      return true;
    }
    return false;
  }

  void AngelCodeFont::load_from_legacy(Reader& reader, const DieselFormatsLoadingParameters& version) { // AngelCodeFont::compile from Lead and Gold
    auto startPosition = reader.GetPosition();

    {// read BlobAllocator header
      auto type = reader.ReadType<uint32_t>();
      if (type != 0xE5BDD873) {
        throw std::runtime_error("Provided font file is not a valid AngelCodeFont file (missing BlobAllocator type)");
      }
      auto version = reader.ReadType<uint32_t>();
      auto original_location = reader.ReadType<uint32_t>();
      auto size = reader.ReadType<uint32_t>(); // size of the blob data, including this 16 byte header
    }

    diesel::InplaceArray<Glyph> glyphs(reader, version);
    diesel::InplaceMap<int, int> character_to_glyph_map(reader, version);
    diesel::InplaceMap<std::pair<int, int>, int> kerning(reader, version);
    diesel::InplaceString face(reader, version);

    this->_size = reader.ReadType<int32_t>();
    this->_texture_width = reader.ReadType<int32_t>();
    this->_texture_height = reader.ReadType<int32_t>();
    this->_base_line = reader.ReadType<int32_t>();
    this->_line_height = reader.ReadType<int32_t>();

    reader.SetPosition(startPosition + glyphs._data);

    for (int i = 0; i < glyphs._n; i++) {
      auto glyph = reader.ReadType<Glyph>();

      this->_glyphs.push_back(glyph);
    }

    /* reading _character_to_glyph_map */
    {
      std::vector<int> keys;
      std::vector<int> values;

      reader.SetPosition(startPosition + character_to_glyph_map._keys._data);
      for (int i = 0; i < character_to_glyph_map._keys._n; i++) { keys.push_back(reader.ReadType<uint32_t>()); /* or char32_t */ }
      reader.SetPosition(startPosition + character_to_glyph_map._values._data);
      for (int i = 0; i < character_to_glyph_map._values._n; i++) { values.push_back(reader.ReadType<uint32_t>()); }

      for (int i = 0; i < keys.size(); i++) {
        this->_character_to_glyph_map.insert({ keys[i], values[i] });
      }
    }

    /* reading _kerning */
    {
      std::vector<std::pair<int, int>> keys;
      std::vector<int> values;

      reader.SetPosition(startPosition + kerning._keys._data);
      for (int i = 0; i < kerning._keys._n; i++) {
        auto key1 = reader.ReadType<uint32_t>();
        auto key2 = reader.ReadType<uint32_t>();
        keys.push_back({ key1, key2 });
      }

      reader.SetPosition(startPosition + kerning._values._data);
      for (int i = 0; i < kerning._values._n; i++) { values.push_back(reader.ReadType<uint32_t>()); }

      for (int i = 0; i < keys.size(); i++) {
        this->_kerning.insert({ keys[i], values[i] });
      }
    }

    reader.SetPosition(startPosition + face._s);
    this->_face = reader.ReadString();
  }

  void AngelCodeFont::load_from_modern(Reader& reader, const DieselFormatsLoadingParameters& version) { // AngelCodeFont::Data::Data from PAYDAY: The Heist v1
    auto startPosition = reader.GetPosition();

    diesel::modern::Vector<Glyph> glyphs(reader, version);
    diesel::modern::SortMap<int, int> character_to_glyph_map(reader, version);
    diesel::modern::SortMap<std::pair<int, int>, int> kerning(reader, version);

    diesel::modern::String face(reader, version);

    this->_size = reader.ReadType<int32_t>();
    this->_texture_width = reader.ReadType<int32_t>();
    this->_texture_height = reader.ReadType<int32_t>();
    this->_base_line = reader.ReadType<int32_t>();
    this->_line_height = reader.ReadType<int32_t>();

    reader.SetPosition(startPosition + glyphs._data);

    for (int i = 0; i < glyphs._size; i++) {
      auto glyph = reader.ReadType<Glyph>();

      this->_glyphs.push_back(glyph);
    }

    reader.SetPosition(startPosition + character_to_glyph_map._data._data);

    for (int i = 0; i < character_to_glyph_map._data._size; i++) {
      auto character = reader.ReadType<uint32_t>(); // or char32_t
      auto glyph = reader.ReadType<uint32_t>();

      this->_character_to_glyph_map.insert({ character, glyph });
    }

    reader.SetPosition(startPosition + kerning._data._data);

    for (int i = 0; i < kerning._data._size; i++) {
      static_assert(sizeof(std::pair<int, int>) == 8);// if this static assert fails, which to reading both values of the pair separately
      auto key = reader.ReadType<std::pair<int, int>>();
      auto kerning = reader.ReadType<int32_t>();

      this->_kerning.insert({ key, kerning });
    }

    reader.SetPosition(startPosition + face._s);

    this->_face = reader.ReadString(); // read is slow if not reading from memory
  }

  std::string AngelCodeFont::DumpFontToXml(const AngelCodeFont& font) { // AngelCodeFont::compile
    std::string xml;

    xml += "<font>\n";

    xml += std::format("\t<info face=\"{}\" size=\"{}\" />\n", font._face, font._size);
    //xml += "  <info face=\"" + font._face + "\" size=\"" + std::to_string(font._size) + "\"/>\n";
    xml += std::format("\t<common lineHeight=\"{}\" base=\"{}\" scaleW=\"{}\" scaleH=\"{}\" />\n", font._line_height, font._base_line, font._texture_width, font._texture_height);
    //xml += "  <common lineHeight=\"" + std::to_string(font._line_height) + "\" base=\"" + std::to_string(font._base_line) + "\" scaleW=\"" + std::to_string(font._texture_width) + "\" scaleH=\"" + std::to_string(font._texture_height) + "\"/>\n";
    xml += "\t<kernings>\n";

    for (auto& kerning : font._kerning) {
      xml += std::format("\t\t<kerning first=\"{}\" seconds=\"{}\" ammount=\"{}\" />\n", kerning.first.first, kerning.first.second, kerning.second);
      //xml += "    <kerning first=\"" + std::to_string(kerning.first.first) + "\" second=\"" + std::to_string(kerning.first.second) + "\" ammount=\"" + std::to_string(kerning.second) + "\"/>\n";
    }
    xml += "\t</kernings>\n";
    xml += "\t<chars>\n";

    for (auto& character : font._character_to_glyph_map) {
      auto& glyph = font._glyphs[character.second];
      xml += std::format("\t\t<char id=\"{}\" x=\"{}\" y=\"{}\" width=\"{}\" height=\"{}\" xadvance=\"{}\" xoffset=\"{}\" yoffset=\"{}\"/>\n", character.first, glyph.x, glyph.y, glyph.w, glyph.h, glyph.xadvance, glyph.xoffset, glyph.yoffset);
      //xml += "    <char id=\"" + std::to_string(character.first) + "\" x=\"" + std::to_string(glyph.x) + "\" y=\"" + "\" width=\"" + "\" height=\"" + "\" xadvance=\"" + "\" xoffset=\"" + "\" yoffset=\"" + "\"/>\n";
    }

    xml += "\t</chars>\n";

    xml += "</font>\n";

    return xml;
  }

  
  FontMakerFont::FontMakerFont(Reader& reader, const DieselFormatsLoadingParameters& version) { // FontMakerFont::FontMakerFont from Lead and Gold
    auto font_version = reader.ReadType<uint32_t>();

    static float texture_width = 0;
    static float texture_height = 0;

    if (font_version == 4) { // FontMakerFont::read_version_4_font_info from Lead and Gold
      uint32_t font_effective_height = reader.ReadType<uint32_t>();
      uint32_t tex_width = reader.ReadType<uint32_t>();
      uint32_t tex_height = reader.ReadType<uint32_t>();
      uint32_t tex_bpp = reader.ReadType<uint32_t>();

      uint16_t start_glyph = reader.ReadType<uint16_t>();
      uint16_t end_glyph = reader.ReadType<uint16_t>();
      uint32_t num_glyphs = reader.ReadType<uint32_t>();

      this->_letters.reserve(num_glyphs);
      this->_glyphs.reserve(num_glyphs);

      for (int i = 0; i < num_glyphs; i++) {
        this->_letters.push_back({ i + (int)start_glyph, i });
      }

      for (int i = 0; i < num_glyphs; i++) {
        GLYPH_ATTR_4 attr{};

        attr.left = reader.ReadType<float>();
        attr.top = reader.ReadType<float>();
        attr.right = reader.ReadType<float>();
        attr.bottom = reader.ReadType<float>();
        attr.wOffset = reader.ReadType<int16_t>();
        attr.wWidth = reader.ReadType<int16_t>();
        attr.wAdvance = reader.ReadType<int16_t>();

        Glyph g{};

        g.x = (int)((double)tex_width * attr.left);
        g.y = (int)((double)tex_height * attr.top);
        g.w = (int)(texture_width * (float)(attr.right - attr.left));
        g.h = (int)(texture_height * (float)(attr.bottom - attr.top));

        g.xadvance = attr.wAdvance;

        this->_glyphs.push_back(g);
      }
    }
    else if (font_version == 5) { // FontMakerFont::read_version_5_font_info from Lead and Gold
      this->_texture_height = 4096;
      this->_texture_width = 4096;

      auto height = reader.ReadType<float>();
      auto top_padding = reader.ReadType<float>();
      auto bottom_padding = reader.ReadType<float>();
      auto height_advance = reader.ReadType<float>();

      this->_size = (int)height;
      this->_line_height = (int)height_advance;

      auto max_glyphs = reader.ReadType<uint16_t>();

      for (int i = 0; i < max_glyphs + 1; i++) {
        auto unk1 = reader.ReadType<uint16_t>(); // or char16_t
        if (unk1) {
          this->_letters.push_back({ i, unk1 });
        }
      }

      auto num_glyphs = reader.ReadType<uint32_t>();

      std::vector<FILE_GLYPH_ATTR> glyphs;
      glyphs.reserve(num_glyphs);


      int sum_width = 0;
      int sum_gh = 0;
      int unk1 = 0;
      for (int i = 0; i < num_glyphs; i++) {
        FILE_GLYPH_ATTR fmg{};

        fmg = reader.ReadType<FILE_GLYPH_ATTR>();

        int v23 = ((this->_texture_width * (float)(fmg.fRight - fmg.fLeft)) + 0.5f);
        unk1 += v23;

        sum_width += fmg.wWidth;
        sum_gh += (int)((this->_texture_height * (float)(fmg.fBottom - fmg.fTop)) + 0.5f);

        glyphs.push_back(fmg);
      }

      // at this point, the file should be read completely

      
      // my best attempt at recreating this math, it is probably wrong
      //this->_texture_width = (int)(((float)this->_texture_width / (((float)unk1 / num_glyphs) / ((float)sum_width / num_glyphs))) + 0.5f);
      //this->_texture_height = (int)(((float)this->_texture_height / ((float)sum_gh / num_glyphs) / this->_size) + 0.5f);

      //this->_glyphs.reserve(num_glyphs);

      // not implemented

      /*for (int i = 0; i < num_glyphs; i++) {
        Glyph g{};

        this->_glyphs.push_back(g);


      }*/
    }
  }
}