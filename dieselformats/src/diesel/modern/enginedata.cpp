#include "diesel/modern/enginedata.h"

#include "diesel/modern/modern_shared.h"
#include "diesel/modern/hash.h"
#include "diesel/modern/hashlist.h"

using namespace diesel;
using namespace diesel::modern;

bool EngineData::Table::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  this->_name = reader.ReadType<uint64_t>();
  auto array = Vector<EngineValue>(reader, version);
  auto hash = SortMap<Idstring, EngineValue>(reader, version);

  auto saved_position = reader.GetPosition(); // position to revert to after data for this table has been read

  reader.SetPosition(array._data);
  for (int i = 0; i < array._size; i++) {
    this->_array.push_back(reader.ReadType<int32_t>());
  }
  reader.SetPosition(hash._data._data);
  for (int i = 0; i < hash._data._size; i++) {
    this->_hash.insert({ reader.ReadType<uint64_t>(), reader.ReadType<uint32_t>() });
  }

  reader.SetPosition(saved_position);
  return true;
}

bool EngineData::Read(Reader& reader, const DieselFormatsLoadingParameters& version) { // dsl::EngineData::EngineData
  if (!VerifyBlobType(reader, 0x7E75CD7A))
    return false;

  auto numbers = BlobSaverChunk(reader, version); // Vector<float>
  auto vector3s = BlobSaverChunk(reader, version); // Vector<Vector3>
  auto quaternions = BlobSaverChunk(reader, version); // Vector<Quaternion>
  auto idstrings = BlobSaverChunk(reader, version); // Vector<Idstring>
  auto tables = BlobSaverChunk(reader, version); // Vector<EngineData::Table>

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
    this->_idstrings.push_back(reader.ReadType<uint64_t>());
  }
  reader.SetPosition(tables._data);
  for (int i = 0; i < tables._size; i++) {
    Table table;
    table.Read(reader, version); // Stride must be 48 bytes!

    this->_tables.push_back(table);
  }

  return true;
}

std::string diesel::modern::EngineData::DumpReferenceToXml(const reference& ref, int indent) {
  std::string xml;

  std::string indent_str;
  for (int i = 0; i < indent; i++) indent_str += "\t";

  if (ref.get_type() == EngineValueType::Table) {
    auto table = ref.get_table();
    std::string table_name = GetGlobalHashlist()->GetIdstringSource(table._name);
    xml += indent_str + "<" + table_name;



    decltype(Table::_hash) cleaned_properties;
    for (auto& property : table._hash) { // remove invalid property entries (invalid type/out of range) due to it being directly saved from a hash map
      if (!IsValidValueType(property.second.get_type()) || ref.get_data()->IsValueOutOfRangeOfArray(property.second))
        continue;
      cleaned_properties.insert(property);
    }

    for (auto& property : cleaned_properties) {
      if (property.second.get_type() == EngineValueType::Table)
        continue;
      xml += std::format(" {}=\"{}\"", GetGlobalHashlist()->GetIdstringSource(property.first), reference(ref.get_data(), property.second).to_string());
    }

    auto& array_entries = table._array;
    if (array_entries.size() == 0) {
      xml += " />\n";
    }
    else {
      xml += ">\n";
    }

    for (auto& entry : array_entries) {
      xml += DumpReferenceToXml(reference(ref.get_data(), entry), indent + 1);
    }

    if (array_entries.size() != 0) {
      xml += indent_str + "</" + table_name + ">\n";
    }
  }

  return xml;
}

bool diesel::modern::EngineData::IsValueOutOfRangeOfArray(EngineValue value) const{
  switch (value.get_type()) {
  case EngineValueType::False:
    return false;
  case EngineValueType::True:
    return false;
  case EngineValueType::Number:
    return value.get_index() >= this->_numbers.size();
  case EngineValueType::Vector3:
    return value.get_index() >= this->_vector3s.size();
  case EngineValueType::Quaternion:
    return value.get_index() >= this->_quaternions.size();
  case EngineValueType::Idstring:
    return value.get_index() >= this->_idstrings.size();
  case EngineValueType::Table:
    return value.get_index() >= this->_tables.size();
  default:
    return true;
  }
}

EngineData::Table EngineData::reference::get_table() const
{
  if (get_type() == EngineValueType::Table)
    return this->_data->_tables[this->get_value().get_index()];
  return EngineData::Table();
}

std::string EngineData::reference::to_string() const {
  switch (get_type()) {
  case EngineValueType::False:
    return "false";
  case EngineValueType::True:
    return "true";
  case EngineValueType::Number:
    return std::format("{}", this->_data->_numbers[this->_value.get_index()]);
  case EngineValueType::Vector3:
  {
    const Vector3& vec = this->_data->_vector3s[this->_value.get_index()];
    return std::format("{} {} {}", vec.x, vec.y, vec.z);
  }
  case EngineValueType::Quaternion:
  {
    const Quaternion& quat = this->_data->_quaternions[this->_value.get_index()];
    return std::format("{} {} {} {}", quat.x, quat.y, quat.z, quat.w);
  }
  case EngineValueType::Idstring:
    return std::string(GetGlobalHashlist()->GetIdstringSource(this->_data->_idstrings[this->_value.get_index()]));
  case EngineValueType::Table:
    return "table";
  default:
    return "nil";
  }
}
