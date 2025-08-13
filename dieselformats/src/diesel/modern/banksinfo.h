#pragma once

#include "diesel/shared.h"

#include "diesel/modern/hash.h"

#include "fileio/reader.h"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace diesel {
  namespace modern {
    class BanksInfo {
    public:
      BanksInfo() = default;

      bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

    private:
      std::vector<std::string> _bank_paths;
      std::map<unsigned int, Idstring> _id_to_entry;
      std::map<Idstring, std::string> _entry_to_name;
    };
  }
}