#include "diesel/modern/strings.h"

#include "diesel/modern/hashlist.h"

using namespace diesel;
using namespace diesel::modern;

bool Strings::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  if (!VerifyBlobType(reader, 0xA2890D53))
    return false;

  // dsl::UnorderedMap<dsl::idstring, dsl::String>
  auto map = BlobSaverChunk(reader, version);

  reader.SetPosition(map._data);

  for (int i = 0; i < map._size; i++) {
    // Stride must be 24 bytes!
    Idstring id = reader.ReadType<uint64_t>();
    auto str = String(reader, version);
    reader.AddPosition(8); // padding?

    auto saved_position = reader.GetPosition();

    reader.SetPosition(str._s);

    auto str_s = reader.ReadString();

    reader.SetPosition(saved_position);

    this->_map.insert({ id, str_s });
  }

  return true;
}

std::string diesel::modern::Strings::DumpStringsToXml(const Strings& strings) {
  std::string xml;

  xml += "<strings>\n"; // should actually be "<stringset>"

  for (auto& entry : strings._map) {
    xml += std::format("\t<string id=\"{}\" value=\"{}\" />\n", GetGlobalHashlist()->GetIdstringSource(entry.first), SanitiseStringForXml(entry.second));
  }

  xml += "</strings>\n";

  return xml;
}
