#include "diesel/lag/stringtable.h"


namespace diesel {
  namespace lag {
    StringTable::StringTable(Reader& reader) { // dsl::StringTable::load
      auto stringCount = reader.ReadType<uint32_t>();

      for (int i = 0; i < stringCount; i++) {
        this->string_vector.push_back(std::string(reader.ReadString()));
      }
    }
    const std::string& StringTable::GetString(StringTableSerialisedType index) const {
      return this->string_vector[index];
    }
  }
}