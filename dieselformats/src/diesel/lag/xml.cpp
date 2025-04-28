#include "xml.h"

#include <cassert>

namespace diesel {
  namespace lag {
    XMLDocument::XMLDocument() {
      this->_root = nullptr;
    }

    XMLDocument::~XMLDocument() {
      if (this->_root)
        delete this->_root;
    }

    bool XMLDocument::ReadFromBinary(Reader& reader, diesel::EngineVersion version) { // dsl::XMLDocument::load_binary (from Lead and Gold)
      char tag[4]{};
      reader.ReadBytesToBuffer(tag, sizeof(tag));
      if (!(tag[0] == 'X' && tag[1] == 'M' && tag[2] == 'L'))
        return false;

      StringTable stringtable(reader);

      this->_root = new XMLNode();
      this->_root->ReadFromBinary(reader, stringtable, version);

      auto dependency_count = reader.ReadType<uint32_t>();

      for (int i = 0; i < dependency_count; i++) {
        this->_dependencies.push_back(reader.ReadString());
      }

      return true;
    }

    std::string XMLDocument::DumpRootToString() {
      std::string str;
      if(this->_root)
        this->_root->DumpNodeAndChildrenToString(str);
      return str;
    }

    XMLNode::XMLNode() {
      this->_type = -1;
      this->_macro = XMLMacro();
    }

    XMLNode::~XMLNode() {
      for (auto child : this->_child_nodes) {
        delete child;
      }
    }

    void XMLNode::ReadFromBinary(Reader& reader, const StringTable& stringtable, diesel::EngineVersion version) {
      this->_type = reader.ReadType<uint32_t>();

      switch (_type) {
      case 2: // value
      {
        auto value_st = reader.ReadType<StringTableSerialisedType>();
        this->_value = stringtable.GetString(value_st);
      }
        break;
      case 1:
      {
        auto name = reader.ReadType<StringTableSerialisedType>();
        this->_name = stringtable.GetString(name);

        auto parameter_count = reader.ReadType<uint32_t>();

        for (int i = 0; i < parameter_count; i++) {
          auto s1 = reader.ReadType<uint32_t>();
          auto s2 = reader.ReadType<uint32_t>();

          auto s1_str = stringtable.GetString(s1);
          auto s2_str = stringtable.GetString(s2);
          this->_parameters.insert({ s1_str, s2_str });
        }

        auto child_node_count = reader.ReadType<uint32_t>();

        for (int i = 0; i < child_node_count; i++) {
          auto node = new XMLNode();
          this->_child_nodes.push_back(node);
          node->ReadFromBinary(reader, stringtable, version);
        }
      }
        break;
      case 4: // XMLMacro
        this->_macro.ReadFromBinary(reader, stringtable, version);
        break;

      default:
        __debugbreak(); // Unknown XMB type
        break;
      }
    }

    std::string GetIndent(int indent, int indent_increment) {
      std::string indented;
      for (int i = 0; i < indent_increment; i++) {
        for (int j = 0; j < indent; j++) {
          indented += " ";
        }
      }
      return indented;
    }
    void XMLNode::DumpNodeAndChildrenToString(std::string& str, int indent, int indent_increment) {
      str += GetIndent(indent, indent_increment) + "<" + this->_name;

      if (this->_parameters.size() != 0) {
        for (auto& param : this->_parameters) {
          str += " " + param.first + "=\"" + param.second + "\"";
        }
      }

      if (this->_child_nodes.size() != 0) {
        str += ">\n";
        for (auto& child : this->_child_nodes) {
          child->DumpNodeAndChildrenToString(str, indent + 1, indent_increment);
        }
        str += GetIndent(indent, indent_increment) + "</" + this->_name + ">\n";
      }
      else {
        str += "/>\n";
      }
    }

    XMLMacro::XMLMacro() {
      this->_expression = false;
    }
    XMLMacro::~XMLMacro() {
    }

    void XMLMacro::ReadFromBinary(Reader& reader, const StringTable& stringtable, diesel::EngineVersion version) {
      this->_name = reader.ReadString();

      auto parameter_count = reader.ReadType<uint32_t>();

      for (int i = 0; i < parameter_count; i++) {
        this->_parameters.push_back(reader.ReadString());
      }

      auto parameter_order_count = reader.ReadType<uint32_t>();

      for (int i = 0; i < parameter_order_count; i++) {
        this->_parameter_order.push_back(reader.ReadType<int>());
      }

      this->_data = reader.ReadString();
      this->_expression = reader.ReadType<bool>();
    }
  }
}

