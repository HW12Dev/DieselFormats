#pragma once

///
/// Functions and classes to read and parse Diesel localisation ".strings" files.
///

#include "diesel/shared.h"

#include "diesel/modern/hash.h"

#include <map>

namespace diesel {
  namespace modern {
    // Localisation format, use "String" for string representations
    class Strings {
    public:
      bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

      std::map<Idstring, std::string>& GetStrings() { return _map; }
      const std::map<Idstring, std::string>& GetStrings() const { return _map; }

      static std::string DumpStringsToXml(const Strings& strings);
    private:
      std::map<Idstring, std::string> _map;
    };
  }
}