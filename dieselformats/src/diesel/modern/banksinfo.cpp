#include "diesel/modern/banksinfo.h"

using namespace diesel;
using namespace diesel::modern;

bool BanksInfo::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  if (!VerifyBlobType(reader, 0xE) && !VerifyBlobType(reader, 0xF)) // PDTH uses 0xE
    return false;

  Vector<String> bank_paths(reader, version);
  SortMap<unsigned int, Idstring> id_to_entry(reader, version);
  SortMap<Idstring, String> entry_to_name(reader, version);

  reader.SetPosition(bank_paths._data);
  for (int i = 0; i < bank_paths._size; i++) {
    String bank_path(reader, version);

    auto saved_position = reader.GetPosition();
    reader.SetPosition(bank_path._s);
    this->_bank_paths.push_back(reader.ReadString());
    reader.SetPosition(saved_position);
  }

  reader.SetPosition(id_to_entry._data._data);
  for (int i = 0; i < id_to_entry._data._size; i++) {
    auto id = reader.ReadType<uint32_t>();

    reader.AddPosition(4); // padding

    Idstring entry = reader.ReadType<uint64_t>();

    this->_id_to_entry.insert({ id, entry });
  }

  reader.SetPosition(entry_to_name._data._data);
  for (int i = 0; i < entry_to_name._data._size; i++) {
    Idstring entry = reader.ReadType<uint64_t>();
    String name(reader, version);

    auto saved_position = reader.GetPosition();
    reader.SetPosition(name._s);
    this->_entry_to_name.insert({ entry, reader.ReadString() });
    reader.SetPosition(saved_position);
  }

  return true;
}

void BanksInfo::Write(Writer& writer, const DieselFormatsLoadingParameters& version) {
  uint64_t positionToWriteBankPathsTo = 0;
  Vector<String>::Write(writer, version, _bank_paths.size(), _bank_paths.size(), positionToWriteBankPathsTo);
  uint64_t positionToWriteIdToEntryTo = 0;
  SortMap<unsigned int, Idstring>::Write(writer, version, _id_to_entry.size(), _id_to_entry.size(), false, positionToWriteIdToEntryTo);
  uint64_t positionToWriteEntryToNameTo = 0;
  SortMap<Idstring, String>::Write(writer, version, _entry_to_name.size(), _entry_to_name.size(), false, positionToWriteEntryToNameTo);

  

  std::map<uint64_t, std::string> bankPathsStringDataLocations;

  uint64_t bankPathsData = writer.GetPosition();
  for (const std::string& path : _bank_paths) {
    uint64_t dataLoc = 0;
    String::Write(writer, version, dataLoc);
    bankPathsStringDataLocations.insert(std::make_pair(dataLoc, path));
  }

  for (auto& locToWrite : bankPathsStringDataLocations) {
    uint64_t pos = writer.GetPosition();

    writer.SetPosition(locToWrite.first);
    writer.WriteType<uint32_t>((uint32_t)pos);
    writer.SetPosition(pos);

    writer.WriteString(locToWrite.second);
  }

  // aligned to 4 bytes/pointer size?
  writer.AlignToSize(4);

  uint64_t idToEntryData = writer.GetPosition();
  for (auto& idToEntry : _id_to_entry) {
    writer.WriteType<uint32_t>(idToEntry.first);
    writer.AddPosition(4); // padding
    writer.WriteType<uint64_t>(idToEntry.second);
  }

  std::map<uint64_t, std::string> entryToNameStringDataLocations;

  uint64_t entryToNameData = writer.GetPosition();
  for (auto& entryToName : _entry_to_name) {
    writer.WriteType<uint64_t>(entryToName.first);

    uint64_t dataLoc = 0;
    String::Write(writer, version, dataLoc);
    entryToNameStringDataLocations.insert(std::make_pair(dataLoc, entryToName.second));
  }

  for (auto& namesToWrite : entryToNameStringDataLocations) {
    uint64_t pos = writer.GetPosition();

    writer.SetPosition(namesToWrite.first);
    writer.WriteType<uint32_t>((uint32_t)pos);
    writer.SetPosition(pos);

    writer.WriteString(namesToWrite.second);
  }

  writer.AlignToSize(4);


  // blob version
  if (version.version > diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST) {
    writer.WriteType<uint32_t>(0xF);
  }
  else {
    writer.WriteType<uint32_t>(0xE);
  }

  writer.SetPosition(positionToWriteBankPathsTo);
  if (diesel::AreLoadParameters32Bit(version)) {
    writer.WriteType<uint32_t>((uint32_t)bankPathsData);
  }
  else {
    writer.WriteType<uint64_t>(bankPathsData);
  }

  writer.SetPosition(positionToWriteIdToEntryTo);
  if (diesel::AreLoadParameters32Bit(version)) {
    writer.WriteType<uint32_t>((uint32_t)idToEntryData);
  }
  else {
    writer.WriteType<uint64_t>(idToEntryData);
  }

  writer.SetPosition(positionToWriteEntryToNameTo);
  if (diesel::AreLoadParameters32Bit(version)) {
    writer.WriteType<uint32_t>((uint32_t)entryToNameData);
  }
  else {
    writer.WriteType<uint64_t>(entryToNameData);
  }

}
