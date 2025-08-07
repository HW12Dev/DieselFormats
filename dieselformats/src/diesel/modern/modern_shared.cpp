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

    void String::Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t& outPositionOfStringPointerInBuffer) {
      AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>(0) : writer.WriteType<uint64_t>(0); // _allocator

      outPositionOfStringPointerInBuffer = writer.GetPosition();

      AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>(0) : writer.WriteType<uint64_t>(0); // _s

    }
    BlobSaverChunk::BlobSaverChunk(Reader& reader, const DieselFormatsLoadingParameters& version) { // dsl::BlobSaver::save_binary/dsl::BlobSaver::save_array
      if(version.version == EngineVersion::RAID_WORLD_WAR_II_LATEST) {
        reader.AddPosition(8);
        this->_size = reader.ReadType<uint64_t>();
        reader.AddPosition(8);
        this->_data = reader.ReadType<uint64_t>();
      }
      else {
        reader.AddPosition(4);
        this->_size = (unsigned long long)reader.ReadType<uint32_t>();
        reader.AddPosition(4); // capacity?
        this->_data = reader.ReadType<uint32_t>();
      }
    }
  }
}
