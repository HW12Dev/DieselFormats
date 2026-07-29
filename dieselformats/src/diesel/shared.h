#pragma once

#include <stdint.h>

#include "fileio/reader.h"
#include "fileio/writer.h"

namespace diesel {
  typedef int EngineVersionBaseType;

  // Engine version / Engine revision
  enum class EngineVersion : EngineVersionBaseType {
    BALLISTICS = 0,
    BANDITS,
    GRAW,
    GRAW2,
    BIONIC_COMMANDO_REARMED,
    BIONIC_COMMANDO,
    WANTED,
    TERMINATOR_SALVATION,
    LEAD_AND_GOLD,
    BIONIC_COMMANDO_REARMED2,

    MODERN_VERSION_START = 10,
    PAYDAY_THE_HEIST_V1,
    PAYDAY_THE_HEIST_LATEST,
    PAYDAY_2_LEGACY, // Legacy PAYDAY 2 bundle format (no allocators)
    PAYDAY_2_LEGACY_CONSOLE, // PAYDAY 2 XBOX 360 / PlayStation 3
    PAYDAY_2_LATEST,
    //PAYDAY_2_XB1_PS4, // XB1, PS4 and Switch might all share 1 base engine version, but they are seperate for now
    //PAYDAY_2_SWITCH,
    PAYDAY_2_MODERN_CONSOLE, // XB1, PS4 and Switch
    PAYDAY_2_LINUX_LATEST,
    RAID_WORLD_WAR_II_LATEST,
    DIESEL_V3 = RAID_WORLD_WAR_II_LATEST
  };

  // Determines which platform a provided file was created for (e.g. PS3 shader database in a Windows build). Used to determine endianness when reading integers.
  enum class FileSourcePlatform : EngineVersionBaseType {
    UNSPECIFIED = 0x0,
    WINDOWS_32 = 0x0, // Little endian
    WINDOWS_64,       // Little endian

    LINUX_64,

    MICROSOFT_XBOX_360, // Big endian https://en.wikipedia.org/wiki/Xenon_(processor)#Specifications
    MICROSOFT_XBOX_ONE,

    SONY_PLAYSTATION_3, // Big endian https://www.psdevwiki.com/ps3/CELL_BE#Specifications
    SONY_PLAYSTATION_4,

    NINTENDO_SWITCH
  };

  enum class Renderer : EngineVersionBaseType {
    UNSPECIFIED = 0x0,
    DIRECTX8,
    DIRECTX9,
    DIRECTX10,
    DIRECTX11,
    OPENGL,
    PLAYSTATION3, // Cell + RSX
    PLAYSTATION4,

    XBOX360 = DIRECTX9,
  };



  // Not from Diesel
  struct DieselFormatsLoadingParameters {
    diesel::EngineVersion version;
    diesel::Renderer renderer;
    diesel::FileSourcePlatform sourcePlatform;

    DieselFormatsLoadingParameters() : version((diesel::EngineVersion)-1), renderer(Renderer::UNSPECIFIED), sourcePlatform(FileSourcePlatform::UNSPECIFIED) {}
    DieselFormatsLoadingParameters(diesel::EngineVersion version) : version(version), renderer(Renderer::UNSPECIFIED), sourcePlatform(FileSourcePlatform::UNSPECIFIED) {}
    DieselFormatsLoadingParameters(diesel::EngineVersion version, diesel::Renderer renderer) : version(version), renderer(renderer), sourcePlatform(FileSourcePlatform::UNSPECIFIED) {}
    DieselFormatsLoadingParameters(diesel::EngineVersion version, diesel::Renderer renderer, diesel::FileSourcePlatform sourcePlatform) : version(version), renderer(renderer), sourcePlatform(sourcePlatform) {}
  };
  bool AreLoadParameters32Bit(const DieselFormatsLoadingParameters& version);
  bool DoLoadParametersHaveIdstrings(const DieselFormatsLoadingParameters& version);
  bool AreLoadParametersForABigEndianPlatform(const DieselFormatsLoadingParameters& version);

  class Quaternion {
  public:
    float x;
    float y;
    float z;
    float w;
  };
  class QuaternionPacked {
  public:
    unsigned int _data;
  };
  class Vector3 {
  public:
    float x;
    float y;
    float z;
  };
  class Vector3Packed {
  public:
    uint16_t x;
    uint16_t y;
    uint16_t z;
  };
  class Vector3d {
  public:
    double x;
    double y;
    double z;
  };
  class Vector4d {
  public:
    double x;
    double y;
    double z;
    double w;
  };

  class Matrix4 {
  public:
    Vector3 x;
    float xw;
    Vector3 y;
    float yw;
    Vector3 z;
    float zw;
    Vector3 t;
    float tw;
  };

  class Matrix4d {
  public:
    Vector4d m0, m1, m2, m3;
  };

  template<typename T>
  class InplaceArray { // used in legacy blob serialisation
  public:
    InplaceArray() : _n(-1), _data(-1) {}
    InplaceArray(Reader& reader, const DieselFormatsLoadingParameters& version);

    static void Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t size, uint64_t& outPositionOfDataPointer);

  public:
    unsigned long long _n; // int
    unsigned long long _data; // T*
  };
  template<typename Key, typename Value>
  class InplaceMap { // used in legacy blob serialisation
  public:
    InplaceMap(Reader& reader, const DieselFormatsLoadingParameters& version);

  public:
    InplaceArray<Key> _keys;
    InplaceArray<Value> _values;
  };

  class InplaceString { // used in legacy blob serialisation
  public:
    InplaceString(Reader& reader, const DieselFormatsLoadingParameters& version);

    static void Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t& outPositionOfDataPointer);

  public:
    unsigned long long _s;
  };

  template<typename T>
  class InplaceVirtual { // used in animation serialisation
  public:
      InplaceVirtual(Reader& reader, const DieselFormatsLoadingParameters& version);

      static void Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint32_t type_id, uint64_t& outPositionOfDataPointer);

  public:
      uint32_t _type_id;
      size_t _object;
  };

  template<typename T>
  InplaceArray<T>::InplaceArray(Reader& reader, const DieselFormatsLoadingParameters& version) {
    if (diesel::AreLoadParameters32Bit(version))
    {
      this->_n = reader.ReadType<uint32_t>();
      this->_data = reader.ReadType<uint32_t>();
    }
    else
    {
      this->_n = reader.ReadType<uint64_t>();
      this->_data = reader.ReadType<uint64_t>();
    }
  }
  template<typename T>
  void InplaceArray<T>::Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t size, uint64_t& outPositionOfDataPointer)
  {
      if (diesel::AreLoadParameters32Bit(version))
      {
          writer.WriteType<uint32_t>(size);
          outPositionOfDataPointer = writer.GetPosition();
          writer.WriteType<uint32_t>(0);
      }
      else
      {
          writer.WriteType<uint64_t>(size);
          outPositionOfDataPointer = writer.GetPosition();
          writer.WriteType<uint64_t>(0);
      }
  }

  template<typename Key, typename Value>
  InplaceMap<Key, Value>::InplaceMap(Reader& reader, const DieselFormatsLoadingParameters& version) {
    this->_keys = InplaceArray<Key>(reader, version);
    this->_values = InplaceArray<Value>(reader, version);
  }

  template<typename T>
  InplaceVirtual<T>::InplaceVirtual(Reader& reader, const DieselFormatsLoadingParameters& version) {
    if (diesel::AreLoadParameters32Bit(version))
    {
      this->_type_id = reader.ReadType<uint32_t>();
      this->_object = reader.ReadType<uint32_t>();
    }
    else
    {
        // total guess
        this->_type_id = reader.ReadType<uint32_t>();
        reader.AddPosition(4);
        this->_object = reader.ReadType<uint64_t>();
    }
  }
  template<typename T>
  void InplaceVirtual<T>::Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint32_t type_id, uint64_t& outPositionOfDataPointer)
  {
      if (diesel::AreLoadParameters32Bit(version))
      {
          writer.WriteType<uint32_t>(type_id);
          outPositionOfDataPointer = writer.GetPosition();
          writer.WriteType<uint32_t>(0);
      }
      else
      {
          writer.WriteType<uint32_t>(type_id);
          writer.AddPosition(4);
          outPositionOfDataPointer = writer.GetPosition();
          writer.WriteType<uint64_t>(0);
      }
  }

  // General utilities
  std::string SanitiseStringForXml(const std::string& str);
  std::string ReplaceInString(std::string str, const std::string& find, const std::string& replace);
  std::wstring ReplaceInString(std::wstring str, const std::wstring& find, const std::wstring& replace);

  bool VerifyBlobType(Reader& reader, uint32_t blobId);

  bool operator==(EngineVersion a, EngineVersion b);
  bool operator!=(EngineVersion a, EngineVersion b);
  bool operator<(EngineVersion a, EngineVersion b);
  bool operator>(EngineVersion a, EngineVersion b);
}