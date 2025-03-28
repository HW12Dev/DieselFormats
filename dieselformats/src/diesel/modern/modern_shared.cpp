#include "diesel/modern/modern_shared.h"

bool operator==(const diesel::modern::ModernEngineVersion a, const diesel::modern::ModernEngineVersion b) {
  return (diesel::EngineVersionBaseType)a == (diesel::EngineVersionBaseType)b;
}

bool operator!=(const diesel::modern::ModernEngineVersion a, const diesel::modern::ModernEngineVersion b) {
  return !(a == b);
}

namespace diesel {
  namespace modern {
      ModernEngineVersion ToModernVersion(diesel::EngineVersion version) {
        diesel::EngineVersionBaseType val = (diesel::EngineVersionBaseType)version;

        if (val < (diesel::EngineVersionBaseType)diesel::EngineVersion::MODERN_VERSION_START)
          return ModernEngineVersion::INVALID_NOT_MODERN;

        return (ModernEngineVersion)(val - (diesel::EngineVersionBaseType)diesel::EngineVersion::MODERN_VERSION_START);
      }
      diesel::EngineVersion ToGenericVersion(ModernEngineVersion version) {
        return (diesel::EngineVersion)((EngineVersionBaseType)version + (EngineVersionBaseType)diesel::EngineVersion::MODERN_VERSION_START);
      }
      bool IsEngineVersion32Bit(ModernEngineVersion version) {
      return version != ModernEngineVersion::RAID_WORLD_WAR_II_LATEST && version != ModernEngineVersion::PAYDAY_2_LINUX_LATEST;
    }

    const char* hex_chars = "0123456789abcdef";

    std::string hex(const char* bytes, int n) {
      char out1[200]{};
      int i;
      char v6;
      char* v7;

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
  }
}
