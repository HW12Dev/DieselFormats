#pragma once

///
/// Functions and classes to read and parse Diesel XMB files. Writing is not currently supported
///

#include "fileio/reader.h"
#include "diesel/lag/stringtable.h"
#include "diesel/shared.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace diesel {
  namespace lag { 
    class XMLMacro { // probably doesn't appear anywhere in any XMB files in public releases. included in case an XMB file contains it
    public:
      XMLMacro();
      ~XMLMacro();

    public:
      void ReadFromBinary(Reader& reader, const StringTable& stringtable, diesel::EngineVersion version);

    private:
      std::string _name;
      std::vector<std::string> _parameters;
      std::vector<int> _parameter_order;
      std::string _data;
      bool _expression;
    };

    class XMLNode {
    public:
      XMLNode();
      ~XMLNode();
    public:
      void ReadFromBinary(Reader& reader, const StringTable& stringtable, diesel::EngineVersion version);

      void DumpNodeAndChildrenToString(std::string& str, int indent = 0, int indent_increment = 2); // not in diesel, added for DieselFormats serialisation

    private:
      int _type;
      std::string _name; // stored as an Idstring in LAG
      std::string _value;

      std::unordered_map<std::string, std::string> _parameters; // key is an Idstring in LAG

      std::vector<XMLNode*> _child_nodes;

      XMLMacro _macro;
    };

    class XMLDocument {
    public:
      XMLDocument();
      ~XMLDocument();

    public:
      /// <summary>
      /// Reads the provided XMB file, returns false if the tag does not match (is not a valid XMB file)
      /// </summary>
      bool ReadFromBinary(Reader& reader, diesel::EngineVersion version);

      std::string DumpRootToString();

    private:
      XMLNode* _root;
      std::vector<std::string> _dependencies;
    };
  }
}