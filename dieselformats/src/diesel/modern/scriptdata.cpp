#include "diesel/modern/scriptdata.h"

using namespace diesel;
using namespace diesel::modern;

bool ScriptData::Table::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  this->meta = reader.ReadType<int>();

  if (!AreLoadParameters32Bit(version))reader.AddPosition(4); // 64bit padding

  auto entries = Vector<TableEntry>(reader, version);

  auto saved_position = reader.GetPosition();

  reader.SetPosition(entries._data);
  for (int i = 0; i < entries._size; i++) {
    this->entries.push_back(reader.ReadType<TableEntry>());
  }

  reader.SetPosition(saved_position);
  return true;
}

bool ScriptData::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  if (!VerifyBlobType(reader, 0x6D75CD70) && !VerifyBlobType(reader, 0x6D75CD7B)) // PDTH uses 0x6D75CD7B
    return false;

  auto numbers = BlobSaverChunk(reader, version);
  auto strings = BlobSaverChunk(reader, version);
  auto vector3s = BlobSaverChunk(reader, version);
  auto quaternions = BlobSaverChunk(reader, version);
  auto idstrings = BlobSaverChunk(reader, version);
  auto tables = BlobSaverChunk(reader, version);

  if (AreLoadParameters32Bit(version)) {
    reader.AddPosition(4);
  }
  else {
    reader.AddPosition(8);
  }

  this->_root = reader.ReadType<EngineData::EngineValue>();

  reader.SetPosition(numbers._data);
  for (int i = 0; i < numbers._size; i++) {
    this->_numbers.push_back(reader.ReadType<float>());
  }
  reader.SetPosition(strings._data);
  for (int i = 0; i < strings._size; i++) {
    String str(reader, version);

    auto saved_position = reader.GetPosition();
    reader.SetPosition(str._s);
    this->_strings.push_back(reader.ReadString());
    reader.SetPosition(saved_position);
  }
  reader.SetPosition(vector3s._data);
  for (int i = 0; i < vector3s._size; i++) {
    this->_vector3s.push_back(reader.ReadType<Vector3>());
  }
  reader.SetPosition(quaternions._data);
  for (int i = 0; i < quaternions._size; i++) {
    this->_quaternions.push_back(reader.ReadType<Quaternion>());
  }
  reader.SetPosition(idstrings._data);
  for (int i = 0; i < idstrings._size; i++) {
    this->_idstrings.push_back(reader.ReadType<Idstring>());
  }
  reader.SetPosition(tables._data);
  for (int i = 0; i < tables._size; i++) {
    Table table;
    table.Read(reader, version);

    this->_tables.push_back(table);
  }

  return true;
}

using EngineValueType = EngineData::EngineValueType;

std::string diesel::modern::ScriptData::GetGenericXmlKey(ScriptData& data, EngineData::EngineValue value) {
  switch (value.get_type()) {
  case EngineValueType::Number:
    return std::format("index=\"{}\"", data._numbers[value.get_index()]);
  case EngineValueType::String:
    return std::format("key=\"{}\"", SanitiseStringForXml(data._strings[value.get_index()]));
  }

  return "";
}

std::string ScriptData::GetGenericXmlValue(ScriptData& data, EngineData::EngineValue value) {
  switch (value.get_type()) {
  case EngineValueType::Nil:
    return "type=\"nil\"";
  case EngineValueType::False:
    return "type=\"boolean\" value=\"false\"";
  case EngineValueType::True:
    return "type=\"boolean\" value=\"true\"";
  case EngineValueType::Number:
    return std::format("type=\"number\" value=\"{}\"", data._numbers[value.get_index()]);
  case EngineValueType::String:
    return std::format("type=\"string\" value=\"{}\"", data._strings[value.get_index()]);
  case EngineValueType::Vector3:
  {
    const Vector3& vec = data._vector3s[value.get_index()];
    return std::format("type=\"vector3\" value=\"{} {} {}\"", vec.x, vec.y, vec.z);
  }
  case EngineValueType::Quaternion:
  {
    const Quaternion& quat = data._quaternions[value.get_index()];
    return std::format("type=\"quaternion\" value=\"{} {} {} {}\"", quat.x, quat.y, quat.z, quat.w);
  }
  case EngineValueType::Idstring:
    return std::format("type=\"idstring\" value=\"{}\"", data._idstrings[value.get_index()].hex());
  case EngineValueType::Table:
    if (data._tables[value.get_index()].meta != -1) {
      return std::format("type=\"table\" metatable=\"{}\"", data._strings[data._tables[value.get_index()].meta]);
    }
    return std::format("type=\"table\"");
  }
  return "";
}

std::string diesel::modern::ScriptData::GetGenericXmlTableEntries(ScriptData& data, Table& table, int indent) {
  std::string xml;

  std::string indent_str = "";
  for (int i = 0; i < indent; i++) indent_str += "\t";

  for (auto& entry : table.entries) {
    xml += indent_str;

    xml += std::format("<entry {} {}", GetGenericXmlKey(data, entry.key), GetGenericXmlValue(data, entry.value));

    if (entry.value.get_type() == EngineValueType::Table) {
      xml += ">\n";
      xml += GetGenericXmlTableEntries(data, data._tables[entry.value.get_index()], indent + 1);
      xml += indent_str;
      xml += "</entry>\n";
    }
    else {
      xml += "/>\n";
    }
  }

  return xml;
}

std::string ScriptData::DumpScriptDataToGenericXml(ScriptData& data) {
  std::string xml;

  xml += std::format("<generic_scriptdata {}>\n", GetGenericXmlValue(data, data._root));

  if (data._root.get_type() == EngineValueType::Table) {
    xml += GetGenericXmlTableEntries(data, data._tables[data._root.get_index()], 1);
  }

  xml += "</generic_scriptdata>";

  return xml;
}
