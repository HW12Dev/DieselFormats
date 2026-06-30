#include "diesel/modern/scriptdata.h"

using namespace diesel;
using namespace diesel::modern;

bool ScriptData::Table::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  this->meta = reader.ReadType<int>();

  if (!AreLoadParameters32Bit(version)) reader.AddPosition(4); // 64bit padding

  auto entries = Vector<TableEntry>(reader, version);

  auto saved_position = reader.GetPosition();

  reader.SetPosition(entries._data);
  for (int i = 0; i < entries._size; i++) {
    this->entries.push_back(reader.ReadType<TableEntry>());
  }

  reader.SetPosition(saved_position);
  return true;
}

void ScriptData::Table::Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t& outTableVectorContentsPosition)
{
    writer.WriteType<int>(meta);

    if (!AreLoadParameters32Bit(version)) writer.AddPosition(4); // 64bit padding

    
    uint64_t entriesPosition = 0;
    Vector<TableEntry>::Write(writer, version, entries.size(), entries.size(), entriesPosition);

    if (entries.empty())
    {
        entriesPosition = 0;
    }

    outTableVectorContentsPosition = entriesPosition;
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

void ScriptData::Write(Writer& writer, const DieselFormatsLoadingParameters& version)
{
    size_t numbersDataPosition = 0;
    size_t stringsDataPosition = 0;
    size_t vector3sDataPosition = 0;
    size_t quaternionsDataPosition = 0;
    size_t idstringsDataPosition = 0;
    size_t tablesDataPosition = 0;

    BlobSaverChunk::Write(writer, version, _numbers.size(), numbersDataPosition);
    BlobSaverChunk::Write(writer, version, _strings.size(), stringsDataPosition);
    BlobSaverChunk::Write(writer, version, _vector3s.size(), vector3sDataPosition);
    BlobSaverChunk::Write(writer, version, _quaternions.size(), quaternionsDataPosition);
    BlobSaverChunk::Write(writer, version, _idstrings.size(), idstringsDataPosition);
    BlobSaverChunk::Write(writer, version, _tables.size(), tablesDataPosition);

    if (AreLoadParameters32Bit(version))
    {
        writer.AddPosition(4);
    }
    else
    {
        writer.AddPosition(8);
    }

    writer.WriteType<EngineData::EngineValue>(_root);

    size_t numbersData = 0;
    size_t stringsData = 0;
    size_t vector3sData = 0;
    size_t quaternionsData = 0;
    size_t idstringsData = 0;
    size_t tablesData = 0;

    numbersData = writer.GetPosition();

    for (size_t i = 0; i < _numbers.size(); i++)
    {
        writer.WriteType<float>(_numbers[i]);
    }

    std::vector<std::pair<std::string, uint64_t>> stringsDataPositions;
    stringsData = writer.GetPosition();

    for (size_t i = 0; i < _strings.size(); i++)
    {
        size_t dataPosition = 0;
        String::Write(writer, version, dataPosition);

        stringsDataPositions.push_back(std::make_pair(_strings[i], dataPosition));
    }

    for (auto& stringsToWrite : stringsDataPositions)
    {
        size_t stringStart = writer.GetPosition();
        writer.WriteString(stringsToWrite.first);

        size_t saved = writer.GetPosition();
        writer.SetPosition(stringsToWrite.second);

        if (AreLoadParameters32Bit(version))
        {
            writer.WriteType<uint32_t>((uint32_t)stringStart);
        }
        else
        {
            writer.WriteType<uint64_t>(stringStart);
        }
        writer.SetPosition(saved);
    }

    writer.AddPosition(3);

    vector3sData = writer.GetPosition();

    for (size_t i = 0; i < _vector3s.size(); i++)
    {
        writer.WriteType<Vector3>(_vector3s[i]);
    }

    quaternionsData = writer.GetPosition();

    for (size_t i = 0; i < _quaternions.size(); i++)
    {
        writer.WriteType<Quaternion>(_quaternions[i]);
    }

    idstringsData = writer.GetPosition();

    for (size_t i = 0; i < _idstrings.size(); i++)
    {
        writer.WriteType<uint64_t>(_idstrings[i]);
    }

    std::map<Table*, uint64_t> tablesDataPositions;
    tablesData = writer.GetPosition();

    for (size_t i = 0; i < _tables.size(); i++)
    {
        uint64_t dataPosition = 0;
        _tables[i].Write(writer, version, dataPosition);

        tablesDataPositions[&_tables[i]] = dataPosition;
    }

    for (auto& tableToWrite : tablesDataPositions)
    {
        if (tableToWrite.second == 0)
        {
            continue;
        }

        size_t tableStart = writer.GetPosition();
        for (size_t i = 0; i < tableToWrite.first->entries.size(); i++)
        {
            writer.WriteType<TableEntry>(tableToWrite.first->entries[i]);
        }

        size_t saved = writer.GetPosition();
        writer.SetPosition(tableToWrite.second);

        if (AreLoadParameters32Bit(version))
        {
            writer.WriteType<uint32_t>((uint32_t)tableStart);
        }
        else
        {
            writer.WriteType<uint64_t>(tableStart);
        }

        writer.SetPosition(saved);
    }

    uint32_t blobType = 0;
    if (version.version <= diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST)
    {
        blobType = 0x6D75CD7B;
    }
    else
    {
        blobType = 0x6D75CD70;
    }
    writer.WriteType<uint32_t>(blobType);

    writer.SetPosition(numbersDataPosition);
    AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>((uint32_t)numbersData) : writer.WriteType<uint64_t>(numbersData);
    writer.SetPosition(stringsDataPosition);
    AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>((uint32_t)stringsData) : writer.WriteType<uint64_t>(stringsData);
    writer.SetPosition(vector3sDataPosition);
    AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>((uint32_t)vector3sData) : writer.WriteType<uint64_t>(vector3sData);
    writer.SetPosition(quaternionsDataPosition);
    AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>((uint32_t)quaternionsData) : writer.WriteType<uint64_t>(quaternionsData);
    writer.SetPosition(idstringsDataPosition);
    AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>((uint32_t)idstringsData) : writer.WriteType<uint64_t>(idstringsData);
    writer.SetPosition(tablesDataPosition);
    AreLoadParameters32Bit(version) ? writer.WriteType<uint32_t>((uint32_t)tablesData) : writer.WriteType<uint64_t>(tablesData);

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
    return std::format("type=\"string\" value=\"{}\"", diesel::SanitiseStringForXml(data._strings[value.get_index()]));
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
