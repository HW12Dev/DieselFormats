#pragma once

#include <stdint.h>

#include "fileio/reader.h"

namespace diesel {
  typedef int EngineVersionBaseType;
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
    PAYDAY_2_LATEST,
    PAYDAY_2_XB1_PS4, // XB1, PS4 and Switch might all share 1 base engine version, but they are seperate for now
    PAYDAY_2_SWITCH,
    PAYDAY_2_LINUX_LATEST,
    RAID_WORLD_WAR_II_LATEST,
  };

  class Vector3 {
  public:
    float x;
    float y;
    float z;
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

  template<typename T>
  class InplaceArray { // used in legacy blob serialisation
  public:
    InplaceArray() : _n(-1), _data(-1) {}
    InplaceArray(Reader& reader, EngineVersion version);

  public:
    unsigned long long _n; // int
    unsigned long long _data; // T*
  };
  template<typename Key, typename Value>
  class InplaceMap { // used in legacy blob serialisation
  public:
    InplaceMap(Reader& reader, EngineVersion version);

  public:
    InplaceArray<Key> _keys;
    InplaceArray<Value> _values;
  };

  class InplaceString { // used in legacy blob serialisation
  public:
    InplaceString(Reader& reader, EngineVersion version);

  public:
    unsigned long long _s;
  };

  template<typename T>
  InplaceArray<T>::InplaceArray(Reader& reader, EngineVersion version) {
    this->_n = reader.ReadType<uint32_t>();
    this->_data = reader.ReadType<uint32_t>();
  }

  template<typename Key, typename Value>
  InplaceMap<Key, Value>::InplaceMap(Reader& reader, EngineVersion version) {
    this->_keys = InplaceArray<Key>(reader, version);
    this->_values = InplaceArray<Value>(reader, version);
  }

  bool operator==(EngineVersion a, EngineVersion b);
  bool operator!=(EngineVersion a, EngineVersion b);
  bool operator<(EngineVersion a, EngineVersion b);
  bool operator>(EngineVersion a, EngineVersion b);
}