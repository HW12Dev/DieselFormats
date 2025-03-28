#pragma once

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

  bool operator==(EngineVersion a, EngineVersion b);
  bool operator!=(EngineVersion a, EngineVersion b);
  bool operator<(EngineVersion a, EngineVersion b);
  bool operator>(EngineVersion a, EngineVersion b);
}