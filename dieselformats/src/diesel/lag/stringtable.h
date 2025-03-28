#pragma once

#include "fileio/reader.h"

#include <vector>
#include <string>

namespace diesel {
  namespace lag {
    typedef uint32_t StringTableSerialisedType;
    class StringTable {
    public:
      StringTable() = default;
      StringTable(Reader& reader);

    public:
      const std::string& GetString(StringTableSerialisedType index) const;

    public:
      std::vector<std::string> string_vector;
    };
  }
}