#pragma once

#include "diesel/shared.h"

#include <vector>
#include <string>

namespace diesel {
  class Animation {
  public:
    bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);
    bool ReadUncompressed(Reader& reader, const DieselFormatsLoadingParameters& version);

  private:
    float _length;
    std::vector<std::string> _names;
    //std::vector<BeatTime> _beat_times;
    //std::vector<AnimationTrigger> _trigger_times;

  };
}