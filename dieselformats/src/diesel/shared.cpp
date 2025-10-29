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

  std::string SanitiseStringForXml(const std::string& str) {
    std::string sanitised;

    for (auto character : str) {
      if (character == '\"') {
        sanitised += "&quot;";
      }
      else if (character == '\'') {
        sanitised += "&apos;";
      }
      else if (character == '<') {
        sanitised += "&lt;";
      }
      else if (character == '>') {
        sanitised += "&gt;";
      }
      else if (character == '&') {
        sanitised += "&amp;";
      }
      else {
        sanitised += character;
      }
    }

    return sanitised;
  }

  std::string ReplaceInString(std::string str, const std::string& find, const std::string& replace)
  {
    size_t findInStr = 0;
    while (findInStr = str.find(find, findInStr), findInStr != std::string::npos) {
      str.replace(findInStr, find.size(), replace);
      findInStr += replace.size();
    }

    return str;
  }

  std::wstring ReplaceInString(std::wstring str, const std::wstring& find, const std::wstring& replace)
  {
    size_t findInStr = 0;
    while (findInStr = str.find(find), findInStr != std::wstring::npos) {
      str.replace(findInStr, find.size(), replace);
    }

    return str;
  }

  bool VerifyBlobType(Reader& reader, uint32_t blobId)
  {
    if (reader.GetFileSize() < 4)
      return false;

    auto startPosition = reader.GetPosition();
    reader.AddPosition(reader.GetFileSize() - 4);

    auto blobType = reader.ReadType<uint32_t>();

    reader.SetPosition(startPosition);
    
    return blobType == blobId;
  }

}

static_assert(sizeof(diesel::Vector3) == 0xC);
static_assert(sizeof(diesel::Quaternion) == 0x10);
static_assert(sizeof(diesel::Matrix4) == 0x40);