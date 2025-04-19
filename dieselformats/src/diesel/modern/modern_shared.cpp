#include "diesel/modern/modern_shared.h"

#include <cassert>

namespace diesel {
  namespace modern {
    const char* hex_chars = "0123456789abcdef";

    std::string hex(const char* bytes, int n) {
      char out1[200]{};
      int i;
      char v6;
      char* v7;

      assert((n * 2) < sizeof(out1) && "Input provided to diesel::modern::hex has too many bytes.");

      char* out = out1;
      for (i = n; i > 0; ++bytes) {
        v6 = *bytes;
        *out = hex_chars[*(unsigned __int8*)bytes >> 4];
        v7 = out + 1;
        *v7 = hex_chars[v6 & 0xF];
        --i;
        out = v7 + 1;
      }

      out1[n * 2] = '\x00';

      return out1;
    }

    String::String(Reader& reader, const DieselFormatsLoadingParameters& version) {
      reader.AddPosition(AreLoadParameters32Bit(version) ? 4 : 8); // _allocator (dsl::Allocator*)

      this->_s = AreLoadParameters32Bit(version) ? reader.ReadType<uint32_t>() : reader.ReadType<uint64_t>();
    }
  }
}
