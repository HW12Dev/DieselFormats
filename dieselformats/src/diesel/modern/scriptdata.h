#pragma once

#include "diesel/shared.h"

#include "diesel/modern/hash.h"

#include "diesel/modern/enginedata.h"

#include <vector>
#include <string>

namespace diesel {
  namespace modern {
    class ScriptData {
      struct TableEntry {
        EngineData::EngineValue key;
        EngineData::EngineValue value;
      };
      class Table {
      public:
        bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);
      public:
        int meta;
        std::vector<TableEntry> entries;
      };
    public:
      bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

    private:
      static std::string GetGenericXmlKey(ScriptData& data, EngineData::EngineValue value);
      static std::string GetGenericXmlValue(ScriptData& data, EngineData::EngineValue value);
      static std::string GetGenericXmlTableEntries(ScriptData& data, Table& table, int indent = 1);
    public:
      static std::string DumpScriptDataToGenericXml(ScriptData& data);

    private:
      std::vector<float> _numbers;
      std::vector<std::string> _strings;
      std::vector<Vector3> _vector3s;
      std::vector<Quaternion> _quaternions;
      std::vector<Idstring> _idstrings;
      std::vector<Table> _tables;

      EngineData::EngineValue _root;
    };
  }
}