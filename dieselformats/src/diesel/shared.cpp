#include "diesel/shared.h"

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
}

static_assert(sizeof(diesel::Vector3) == 0xC);
static_assert(sizeof(diesel::Matrix4) == 0x40);