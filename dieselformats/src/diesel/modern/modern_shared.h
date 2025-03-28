#pragma once

#include <string>

#include "diesel/shared.h"

namespace diesel {
  namespace modern {
    enum class ModernEngineVersion : EngineVersionBaseType {
      INVALID_NOT_MODERN = 0, // used for conversion between the generic EngineVersion enum
      PAYDAY_THE_HEIST_V1 = 1,
      PAYDAY_THE_HEIST_LATEST,
      PAYDAY_2_LATEST,
      PAYDAY_2_XB1_PS4, // XB1, PS4 and Switch might all share 1 base engine version, but they are seperate for now
      PAYDAY_2_SWITCH,
      PAYDAY_2_LINUX_LATEST,
      RAID_WORLD_WAR_II_LATEST
    };

    ModernEngineVersion ToModernVersion(diesel::EngineVersion version);
    diesel::EngineVersion ToGenericVersion(ModernEngineVersion version);

    bool IsEngineVersion32Bit(ModernEngineVersion version);

    std::string hex(const char* bytes, int n);
    
  }
}
bool operator==(const diesel::modern::ModernEngineVersion a, const diesel::modern::ModernEngineVersion b);
bool operator!=(const diesel::modern::ModernEngineVersion a, const diesel::modern::ModernEngineVersion b);