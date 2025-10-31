#include "misc.h"

#include <sstream>

using namespace diesel::modern;

double diesel::modern::DigestValueToNumber(const char* digestValue) { return DigestValueToNumber(std::string(digestValue)); }
double diesel::modern::DigestValueToNumber(const std::string& digestValue) {
  std::string digest;

  digest.resize(digestValue.size() / 2);

  for (unsigned int i = 0; i < digest.size(); i++) {
    char c = -2 - digestValue[i * 2];
    digest[i] = c;
  }

  return std::stod(digest);

}

std::string diesel::modern::NumberToDigestValue(float num) { return NumberToDigestValue((double)num); }
std::string diesel::modern::NumberToDigestValue(double num) {
  std::stringstream sstream;
  sstream << num;
  std::string numStr = sstream.str();

  std::string digest;
  digest.resize(numStr.size() * 2);

  for (unsigned int i = 0; i < numStr.size(); i++) {
    char c = -2 - numStr[i];
    digest[i * 2] = c;
    digest[i * 2 + 1] = (i + 731 * (i + 1)) % 0xFE;
  }

  return digest;
}
