#pragma once

#include <string>

namespace diesel {
  namespace modern {
    double DigestValueToNumber(const char* str);
    double DigestValueToNumber(const std::string& digestValue);

    std::string NumberToDigestValue(float num);
    std::string NumberToDigestValue(double num);
  }
}