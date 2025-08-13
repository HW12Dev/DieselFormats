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
