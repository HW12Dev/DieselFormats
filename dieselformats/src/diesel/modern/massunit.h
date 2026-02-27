#pragma once

#include "diesel/shared.h"

#include "diesel/modern/hash.h"

#include <vector>

namespace diesel {
  namespace modern {
    class MassUnitResource {
    private:
      struct InstanceData {
        Vector3 pos;
        Quaternion rot;
      };
      struct UnitTypeData {
        Idstring unit_type;
        int pool_size;
        std::vector<InstanceData> instances;
      };

    public:
      MassUnitResource();

      bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

      std::vector<UnitTypeData> _types;
    };
  }
}