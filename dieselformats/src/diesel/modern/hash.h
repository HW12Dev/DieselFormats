#pragma once

#include "diesel/modern/modern_shared.h"
#include <string>

namespace diesel {
  namespace modern {
    class Idstring {
    public:
      Idstring();
      Idstring(const std::string& str);
      Idstring(const char* str, unsigned long long length);
      Idstring(unsigned long long id);

      Idstring(const Idstring& other);

    public:
      bool operator==(const Idstring& other) const;
      bool operator!=(const Idstring& other) const;
      bool operator<(const Idstring& other) const;
      bool operator>(const Idstring& other) const;

      operator unsigned long long() const;

      std::string hex() const;

    private:
      unsigned long long _id;
    };

    unsigned long long hash64(
      char* k,     /* the key */
      unsigned long long length, /* the length of the key */
      unsigned long long level  /* the previous hash, or an arbitrary value */
    );
    unsigned long long hash64(const std::string& str);
  }
}