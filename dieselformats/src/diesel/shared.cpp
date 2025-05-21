#include "diesel/shared.h"

#include "fileio/reader.h"

namespace diesel {
  bool operator==(EngineVersion a, EngineVersion b) {
    return (EngineVersionBaseType)a == (EngineVersionBaseType)b;
  }
  bool operator!=(EngineVersion a, EngineVersion b) {
    return !(a == b);
  }
  bool operator<(EngineVersion a, EngineVersion b) {
    return (EngineVersionBaseType)a < (EngineVersionBaseType)b;
  }
  bool operator>(EngineVersion a, EngineVersion b) {
    return (EngineVersionBaseType)a > (EngineVersionBaseType)b;
  }
  bool AreLoadParameters32Bit(const DieselFormatsLoadingParameters& version) {
    return (version.sourcePlatform != diesel::FileSourcePlatform::WINDOWS_64 && version.sourcePlatform != diesel::FileSourcePlatform::LINUX_64) && version.version != EngineVersion::RAID_WORLD_WAR_II_LATEST || version.version == EngineVersion::PAYDAY_2_LEGACY_CONSOLE;
    //return version != EngineVersion::RAID_WORLD_WAR_II_LATEST && version != EngineVersion::PAYDAY_2_LINUX_LATEST;
  }

  bool DoLoadParametersHaveIdstrings(const DieselFormatsLoadingParameters& version) {
    return version.version > diesel::EngineVersion::MODERN_VERSION_START;
  }

  bool AreLoadParametersForABigEndianPlatform(const DieselFormatsLoadingParameters& version) {
    return version.sourcePlatform == diesel::FileSourcePlatform::SONY_PLAYSTATION_3 || version.sourcePlatform == diesel::FileSourcePlatform::MICROSOFT_XBOX_360;
  }

  InplaceString::InplaceString(Reader& reader, const DieselFormatsLoadingParameters& version) {
    this->_s = reader.ReadType<uint32_t>();
  }

}

static_assert(sizeof(diesel::Vector3) == 0xC);
static_assert(sizeof(diesel::Matrix4) == 0x40);