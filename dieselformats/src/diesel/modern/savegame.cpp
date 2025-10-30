#include "diesel/modern/savegame.h"

#include <cassert>

using namespace diesel;
using namespace diesel::modern;

static const int SAVEGAME_BLOB_COUNT = 7;
/**
* SaveGame blob order:
* 
* 1. HeaderData
* 2. InformationData
* 3. UnitData
* 4. SmallIconData (RawData)
* 5. LargeIconData (RawData)
* 6. AnimationData (RawData)
* 7. SoundData (RawData)
*/

bool SaveGame::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  reader.SetPosition(4);

  std::vector<Reader> blobs;

  for (int i = 0; i < SAVEGAME_BLOB_COUNT; i++) {
    auto blobSize = reader.ReadType<uint32_t>();
    char* blobData = new char[blobSize];
    memset(blobData, 0, blobSize);
    reader.ReadBytesToBuffer(blobData, blobSize);

    blobs.push_back(Reader(blobData, blobSize));

  }

  char saveGameMD5[16]{};
  reader.ReadBytesToBuffer(saveGameMD5, sizeof(saveGameMD5));

  assert(reader.AtEndOfBuffer());

  if (blobs[1].GetFileSize() > 4) {
    informationData.Read(blobs[1], version);
  }

  return true;
}

///
/// Worst code I've ever written. Diesel reads save data directly to a Lua table, but this needs to save it to a C++ structure since Lua isn't used here.
///

bool SaveGame::InformationData::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  reader.SetPosition(4); // only set to 4 if it "has verification"

  ReadValue(reader, version, value);

  return true;
}

bool SaveGame::InformationData::SerializerScriptValueVariant::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  auto typeId = reader.ReadType<ScriptValueTypeId>();

  this->type = typeId;

  switch (typeId) {
  case ScriptValueTypeId::ScriptVector3:
    this->vector3Value = reader.ReadType<Vector3>();
    break;
  case ScriptValueTypeId::ScriptQuaternion:
    this->quaternionValue = reader.ReadType<Quaternion>();
    break;
  case ScriptValueTypeId::ScriptPolar:
    reader.ReadBytesToBuffer(&this->polarValue, sizeof(polarValue));
    break;
  case ScriptValueTypeId::ScriptColor:
    reader.ReadBytesToBuffer(&this->colorValue, sizeof(colorValue));
    break;
  case ScriptValueTypeId::ScriptIdstring:
    this->idstringValue = reader.ReadType<uint64_t>();
    break;
  case ScriptValueTypeId::SlotMask:
    reader.ReadBytesToBuffer(&this->slotMaskValue, sizeof(slotMaskValue));
    break;
  default:
    throw std::runtime_error("Unknown Script Value Type Id: " + std::to_string((uint32_t)typeId));
  }

  return true;
}

void SaveGame::InformationData::ReadValue(Reader& reader, const DieselFormatsLoadingParameters& version, SerializerVariant& returnValue) { // dsl::Serializer::read
  SerializerTypeId type = SerializerTypeId::_TYPE_ID_NIL;
  if (version.version > EngineVersion::PAYDAY_THE_HEIST_LATEST) {
    type = reader.ReadType<SerializerTypeId>(); // type is an 8bit enum in post PDTH
  }
  else {
    enum PDTHSerializerTypeId : uint32_t {
      TYPE_ID_NIL = 0x852438D0,
      TYPE_ID_STRING = 0xB45CFFE0,
      TYPE_ID_NUMBER = 0xB1BC248A,
      TYPE_ID_BOOLEAN = 0x84E2C64F,
      TYPE_ID_TABLE = 0xAAB9E1DE,
      TYPE_ID_FUNCTION = 0xC1C42526,
      TYPE_ID_VALUE = 0x2063C160,
      TYPE_ID_REFERENCE = 0xB8AF13EA,
    };

    PDTHSerializerTypeId pdthType = reader.ReadType<PDTHSerializerTypeId>(); // type is a 32bit enum in PDTH

    switch (pdthType) {
    case TYPE_ID_NIL:
      type = SerializerTypeId::_TYPE_ID_NIL;
      break;

    case TYPE_ID_STRING:
      type = SerializerTypeId::_TYPE_ID_STRING;
      break;
    case TYPE_ID_NUMBER:
      type = SerializerTypeId::_TYPE_ID_NUMBER;
      break;

    case TYPE_ID_BOOLEAN:
      type = SerializerTypeId::_TYPE_ID_BOOLEAN;
      break;
    case TYPE_ID_TABLE:
      type = SerializerTypeId::_TYPE_ID_TABLE;
      break;

    case TYPE_ID_VALUE:
      type = SerializerTypeId::_TYPE_ID_SCRIPTVALUE;
      break;

    case TYPE_ID_REFERENCE:
      type = SerializerTypeId::_TYPE_ID_SCRIPTREFERENCE;
      break;

    case TYPE_ID_FUNCTION:
      throw std::runtime_error("TYPE_ID_FUNCTION encountered when loading PAYDAY: The Heist era save file");
      break;

    default:
      __debugbreak(); // Unknown PDTH-era save file type id encountered.
    }
  }



  returnValue.SetType(type);
  switch (type) {

  case SerializerTypeId::_TYPE_ID_STRING:
  {
    returnValue.GetString() = reader.ReadString();
  }
  break;
  case SerializerTypeId::_TYPE_ID_NUMBER:
  {
    returnValue.GetFloat() = reader.ReadType<float>();
  }
  break;
  case SerializerTypeId::_TYPE_ID_ZERO:
  {
    returnValue.GetFloat() = 0.0f;
  }
  break;
  case SerializerTypeId::_TYPE_ID_INT8:
  {
    returnValue.GetInt() = reader.ReadType<uint8_t>();
  }
  break;
  case SerializerTypeId::_TYPE_ID_INT16:
  {
    returnValue.GetInt() = reader.ReadType<uint16_t>();
  }
  break;
  case SerializerTypeId::_TYPE_ID_BOOLEAN:
  {
    returnValue.GetInt() = reader.ReadType<uint8_t>();
  }
  break;
  case SerializerTypeId::_TYPE_ID_TABLE:
  {
    ReadTable(reader, version, returnValue);
  }
  break;

  case SerializerTypeId::_TYPE_ID_SCRIPTVALUE:
  {
    returnValue.GetScriptValue().Read(reader, version);

  }
  break;

  }
}

void SaveGame::InformationData::ReadTable(Reader& reader, const DieselFormatsLoadingParameters& version, SerializerVariant& returnTable) { // dsl::Serializer::read_table
  auto numEntries = reader.ReadType<uint32_t>();

  auto& table = returnTable.GetTable();

  for (int i = 0; i < numEntries; i++) {
    auto newEntry = std::make_pair(SerializerVariant(), SerializerVariant());

    ReadValue(reader, version, newEntry.first);
    ReadValue(reader, version, newEntry.second);

    table.insert(newEntry);
  }
}

bool diesel::modern::SaveGame::InformationData::SerializerVariant::operator<(const SerializerVariant& other) const {
  std::string cmp1;
  std::string cmp2;

  if (this->GetType() == SerializerTypeId::_TYPE_ID_STRING) {
    cmp1 = this->GetString();
  }
  else if (this->GetType() >= SerializerTypeId::_TYPE_ID_ZERO && this->GetType() <= SerializerTypeId::_TYPE_ID_BOOLEAN) {
    cmp1 = std::to_string(this->GetInt());
  }

  if (other.GetType() == SerializerTypeId::_TYPE_ID_STRING) {
    cmp2 = other.GetString();
  }
  else if (other.GetType() >= SerializerTypeId::_TYPE_ID_ZERO && other.GetType() <= SerializerTypeId::_TYPE_ID_BOOLEAN) {
    cmp2 = std::to_string(this->GetInt());
  }

  return cmp1 < cmp2;
}
