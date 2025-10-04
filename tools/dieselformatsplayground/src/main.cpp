#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "fileio/reader.h"
#include "fileio/zlibcompression.h"
#include "diesel/graw/dieselscript.h"
#include "diesel/lag/xml.h"
#include "diesel/lag/linearfile.h"
#include "diesel/modern/bundle.h"
#include "diesel/font.h"
#include "diesel/modern/hashlist.h"
#include "diesel/objectdatabase.h"
#include "diesel/objectdatabase_model.h"
#include "diesel/oil.h"
#include "diesel/random.h"
#include "diesel/modern/enginedata.h"
#include "diesel/modern/scriptdata.h"
#include "diesel/modern/strings.h"
#include "diesel/modern/banksinfo.h"

#include <iostream>
#include <fstream>
#include <set>
#include <cassert>
#include <functional>
#include <utility>
#include <unordered_set>
#include <inttypes.h>

using namespace diesel::modern;
using namespace diesel::objectdatabase::typeidclasses;

#pragma region Testing Functions

void replaceinstr(std::string& in, std::string find, std::string replace) {
  std::string::size_type n = 0;
  while ((n = in.find(find, n)) != std::string::npos) {
    in.replace(n, find.size(), replace);
    n += replace.size();
  }
}

const char* LuaDecryptionKey = "asljfowk4_5u2333crj";
//const char* LuaDecryptionKey = ">S4?fo%k4_5u2(3_=cRj";
auto LuaDecryptionKeyLen = strlen(LuaDecryptionKey);

void copyfile(std::string source, std::string dest, bool lua) {

  if (!lua) {
    std::ifstream in(source, std::ios::binary);
    std::ofstream out(dest, std::ios::binary);

    out << in.rdbuf();
    out.close();
    in.close();
  }
  else {
    Reader file(source);
    auto fs = file.GetFileSize();
    char* buffer = new char[fs];
    file.ReadBytesToBuffer(buffer, fs);
    file.Close();


    for (int i = 0; i < fs; i++) {
      //int keyIndex = ((fs + i) * 7) % LuaDecryptionKeyLen;
      //buffer[i] ^= (char)(LuaDecryptionKey[keyIndex] * (fs - i));
      buffer[i] ^= LuaDecryptionKey[i % LuaDecryptionKeyLen];
    }

    std::ofstream out(dest, std::ios::binary);
    out.write(buffer, fs);
    out.close();

    delete[] buffer;

  }
}

void AddDummyPackageFileToRaid() {
  auto bundleDbPath = "X:\\Projects\\DieselEngineExplorer\\bundle_db.blb";
  //auto bundleDbPath = "X:\\SteamLibrary\\steamapps\\common\\PAYDAY The Heist\\assets\\all.blb";
  Reader bdbreader(bundleDbPath);
  diesel::modern::BundleDatabase bdb;
  bdb.Read(bdbreader, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

  unsigned int nextDBKey = 0;

  for (auto& entry : bdb.GetLookup()) {
    if (nextDBKey < entry.second)
      nextDBKey = entry.second;
  }

  nextDBKey++;

  std::vector<std::pair<std::string, std::string>> to_add = { {"core/packages/base", "package"}, {"packages/boot_screen", "package"} };

  for (auto& add : to_add) {
    diesel::modern::DBExtKey newKey{};
    newKey._type = diesel::modern::Idstring(add.second);
    newKey._name = diesel::modern::Idstring(add.first);
    newKey._properties = 0;

    std::cout << "Adding " << add.first << "." << add.second << " at db key: " << nextDBKey << "\n";

    bdb.AddFile(newKey, nextDBKey);
    nextDBKey++;
  }

  auto outBundleDbPath = "X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assets\\all.blb";
  Writer modifiedBdb(outBundleDbPath);
  bdb.Write(modifiedBdb, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

}

void RemakePackageXml(const std::vector<diesel::modern::ResourceID>& package, std::string outFilePath, bool fonts_only) {
  //auto bundleDbPath = "X:\\Projects\\DieselEngineExplorer\\all.blb";
  //Reader bdbreader(bundleDbPath);
  //diesel::modern::BundleDatabase bdb(bdbreader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);

  if (!std::filesystem::exists(std::filesystem::path(outFilePath).parent_path()))
    std::filesystem::create_directories(std::filesystem::path(outFilePath).parent_path());

  std::ofstream outFile(outFilePath);

  outFile << "<package>\n";

  //outFile << "\t<streaming win32=\"all\" xb1=\"all\" ps4=\"all\"/>\n"; // formats supports "{single_format}", "{format1} {format2}..." and "all"

  Hashlist* hashlist = GetGlobalHashlist();

  outFile << "\t<packageassets>\n";
  for (auto& entry : package) {
    outFile << std::format("\t\t<{} name=\"{}\" />\n", hashlist->GetIdstringSource(entry.type), hashlist->GetIdstringSource(entry.name));
  }
  outFile << "\t</packageassets>\n";

  outFile << "</package>\n";

  outFile.close();
}

void DumpPackage(const std::filesystem::path& path) {
  Reader reader1(path);

  diesel::modern::PackageBundle package(path, reader1, diesel::EngineVersion::PAYDAY_2_LATEST);

  std::string pkgName = "";

  auto packageName = std::stoull(path.filename().replace_extension("").string().substr(0, 16), 0, 16);

  if (!diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(diesel::modern::Idstring(_byteswap_uint64(packageName)), pkgName)) {
    std::cout << "Couldn't find name for " << path.string() << std::endl;
    //return;
  }

  RemakePackageXml(package.GetResources(), "./pd2packages/" + pkgName + ".package", false);
}

#pragma endregion


/*namespace std {
  template<>
  struct hash<Idstring> {
    std::size_t operator()(Idstring const& p) const
    {
      return p.operator unsigned long long();
    }
  };
}

namespace std {
  template<>
  struct hash<std::pair<Idstring, Idstring>> {
    std::size_t operator()(std::pair<Idstring, Idstring> const& p) const
    {
      return p.first ^ p.second;
    }
  };
}*/

using namespace diesel::modern;

void bc2_xb360_extract_test() {
  //std::filesystem::path inputDirectory = "X:\\Projects\\DieselEngineExplorer\\test_files\\PS3 PD2";
  //std::filesystem::path outputDirectory = "X:\\Projects\\DieselEngineExplorer\\test_files\\PS3 PD2 Unpack";
  //std::filesystem::path inputDirectory = "E:\\dieselgames\\Bionic Commando - Rearmed 2 (World) (XBLA)\\unpack\\assets";
  //std::filesystem::path outputDirectory = "E:\\dieselgames\\Bionic Commando - Rearmed 2 (World) (XBLA)\\unpack\\assetsunpack";
  std::filesystem::path inputDirectory = R"(X:\Projects\DieselEngineExplorer\test_files\pd2_ps4\ps4pkg\PAYDAY 2 - CRIMEWAVE EDITION\Image0\assets)";

  std::filesystem::path outputDirectory = R"(X:\Projects\DieselEngineExplorer\test_files\pd2_ps4\ps4pkg\PAYDAY 2 - CRIMEWAVE EDITION\Image0\assetsunpack)";

  //diesel::DieselFormatsLoadingParameters loadingParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::BIONIC_COMMANDO_REARMED2, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::MICROSOFT_XBOX_360);
  //diesel::DieselFormatsLoadingParameters loadingParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::SONY_PLAYSTATION_3);
  diesel::DieselFormatsLoadingParameters loadingParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::SONY_PLAYSTATION_4);

  Reader bdbReader(inputDirectory / "bundle_db.blb");
  bdbReader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(loadingParams));
  BundleDatabase bdb;

  printf("Loading Bundle Database\n");

  bdb.Read(bdbReader, loadingParams);

  std::set<ResourceID> bundleDatabaseAssets;

  for (auto& entry : bdb.GetLookup()) {
    bundleDatabaseAssets.insert(ResourceID{ .type = entry.first._type, .name = entry.first._name });
  }

  printf("Loaded Bundle Database. Lookup Entries: %" PRIu64 "\n", bundleDatabaseAssets.size());

  printf("Loading streamed bundle\n");
  Bundle bndl(inputDirectory, "all", loadingParams);

  uint64_t bundleAssets = 0;

  for (auto& header : bndl.GetHeaders()) {
    bundleAssets += header->size();
  }

  printf("Loaded streamed bundle. Assets: %" PRIu64 "\n", bundleAssets);

  std::vector<PackageBundle*> packages;

  printf("Loading packages\n");

  for (std::filesystem::recursive_directory_iterator i(inputDirectory), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h") == std::string::npos)
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;
      if (i->path().string().find("stream") != std::string::npos)
        continue;


      Reader reader1(i->path());
      reader1.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(loadingParams));
      PackageBundle* package = new PackageBundle(i->path(), reader1, loadingParams);

      packages.push_back(package);
    }
  }

  printf("Loaded packages.\n");

  Hashlist* hashlist = diesel::modern::GetGlobalHashlist();

  int idstring_lookup_db_key = bdb.GetDBKeyFromTypeAndName(Idstring("idstring_lookup"), Idstring("idstring_lookup"));

  if (idstring_lookup_db_key != -1) {
    Reader idstringLookup;
    bool opened = bndl.open(idstringLookup, idstring_lookup_db_key);
    if (opened) {
      hashlist->ReadFileToHashlist(idstringLookup);
    }
  }

  std::map<int, std::string> properties;

  for (auto& prop : bdb.GetProperties()) {
    properties.insert({ prop.second, hashlist->GetIdstringSource(prop.first) });
  }

  printf("Extracting...\n");

  uint64_t filesExtracted = 0;

  for (auto& entry : bdb.GetLookup()) {
    bool foundEntry = false;
    Reader entryContents;

    foundEntry = bndl.open(entryContents, entry.second);
    if (!foundEntry) {
      for (auto package : packages) {
        foundEntry = package->open(entryContents, entry.second);

        if (foundEntry)
          break;
      }
    }
    if (!foundEntry) {
      continue; // Packages and streamed bundles do not contain this file, skip it.
    }

    std::string outputFileName = hashlist->GetIdstringSource(entry.first._name);

    int entryProperties = entry.first._properties;
    if (entryProperties != 0) {
      for (auto& prop : properties) {
        if ((entryProperties & prop.first) != 0) {
          outputFileName += "." + prop.second;
        }
      }
    }

    std::filesystem::path entryOutPath = outputDirectory / (outputFileName + "." + hashlist->GetIdstringSource(entry.first._type));

    if(!std::filesystem::exists(entryOutPath.parent_path()))
      std::filesystem::create_directories(entryOutPath.parent_path());

    try {
      Writer outWriter(entryOutPath);
      outWriter.WriteReader(entryContents);
      outWriter.Close();
      filesExtracted++;
    }
    catch (std::runtime_error err) {
      std::cout << err.what() << std::endl;
    }

  }

  printf("Successfully extracted %" PRIu64 " files\n", filesExtracted);

  for (auto package : packages) {
    delete package;
  }
}


void undatacompile(const std::filesystem::path& inPackedAssets, const std::filesystem::path& inoutUnpackedAssets, diesel::EngineVersion version) {

  Hashlist* hashlist = GetGlobalHashlist();

  for (std::filesystem::recursive_directory_iterator i(inPackedAssets), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h.bundle") != std::string::npos)
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;

      auto package = Idstring(_byteswap_uint64(std::stoull(i->path().filename().replace_extension(), nullptr, 16)));

      if (package == Idstring("engine-package") || package == Idstring("lua-package"))
        continue;

      auto packageHeader = inPackedAssets / (package.hex() + "_h.bundle");
      Reader packageBundleR(packageHeader);
      PackageBundle packageBundle(packageHeader, packageBundleR, version);
      packageBundleR.Close();

      std::ofstream outPackageXml(inoutUnpackedAssets / (hashlist->GetIdstringSource(package) + ".package"));

      outPackageXml << "<package>\n";
      outPackageXml << "\t<resources>\n";

      for (auto& resource : packageBundle.GetResources()) {
        outPackageXml << std::format("\t\t<{} name=\"{}\" />\n", hashlist->GetIdstringSource(resource.type), hashlist->GetIdstringSource(resource.name));
      }

      outPackageXml << "\t</resources>\n";
      outPackageXml << "</package>";
    }
  }

  std::set<std::string> scriptDataExtensions = {
    ".continents",
    ".cover_data",
    ".mission",
    ".nav_data",
    ".world_cameras",
    ".world_sounds",
    ".world",
    ".continent",

    ".achievment",
    ".action_message",
    ".comment",
    ".credits",
    ".hint",
    ".objective",
    ".dialog",
    ".dialog_index",
    ".menu",

    ".prefhud",
    ".sequence_manager"
  };

  for (std::filesystem::recursive_directory_iterator i(inoutUnpackedAssets), end; i != end; ++i) {
    if (std::filesystem::is_directory(i->path()))
      continue;

    auto path = i->path();
    auto extension = path.extension();

    if (scriptDataExtensions.contains(extension.string())) {
      Reader fileReader(path);
      ScriptData engineData;
      bool success = engineData.Read(fileReader, version);
      fileReader.Close();

      if (success) {
        std::ofstream outEngineData(path);
        outEngineData << ScriptData::DumpScriptDataToGenericXml(engineData);
        outEngineData.close();
      }
    }
    else if (extension == ".render_config") {
      Reader fileReader(path);
      EngineData engineData;
      bool success = engineData.Read(fileReader, version);
      fileReader.Close();

      if (success) {
        std::ofstream outEngineData(path);
        outEngineData << EngineData::DumpReferenceToXml(engineData.GetRoot());
        outEngineData.close();
      }
    }
    else if (extension == ".font") {
      Reader fileReader(path);
      diesel::AngelCodeFont engineData;
      bool success = engineData.Read(fileReader, version);
      fileReader.Close();

      if (success) {
        std::ofstream outEngineData(path);
        outEngineData << diesel::AngelCodeFont::DumpFontToXml(engineData);
        outEngineData.close();
      }
    }
    else if (extension == ".strings") {
      Reader fileReader(path);
      Strings engineData;
      bool success = engineData.Read(fileReader, version);
      fileReader.Close();

      if (success) {
        std::ofstream outEngineData(path);
        outEngineData << Strings::DumpStringsToXml(engineData);
        outEngineData.close();
      }
    }
  }
}

bool open_package(std::vector<PackageBundle*>& packages, unsigned int dbKey, Reader& reader)
{
  for (auto package : packages) {
    if (package->open(reader, dbKey))
      return true;
  }
  return false;
}

std::vector<PackageBundle*> open_packages(const char* dir)
{
  std::vector<PackageBundle*> packages;
  for (std::filesystem::recursive_directory_iterator i(dir), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h.bundle") == std::string::npos)
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;


      Reader reader1(i->path());
      PackageBundle* package = new PackageBundle(i->path(), reader1, diesel::EngineVersion::PAYDAY_2_LATEST);


      packages.push_back(package);
    }
  }

  return packages;
}

struct EntryInfo {
  unsigned int size;
  unsigned int offset;
};

class Payday2StreamedBundleHeader {
public:
  Payday2StreamedBundleHeader() {};

public:
  void Read(Reader& reader, const diesel::DieselFormatsLoadingParameters& version);
  void Write(Writer& writer, const diesel::DieselFormatsLoadingParameters& version);

public:
  std::vector<Bundle::HeaderVectorType*> _headers;
  std::map<Bundle::HeaderVectorType*, int> _header_indices;
};


void Payday2StreamedBundleHeader::Read(Reader& reader, const diesel::DieselFormatsLoadingParameters& version)
{
  auto headers = BlobSaverChunk(reader, version);

  reader.SetPosition(headers._data);

  reader.AddPosition(4);
  for (int i = 0; i < headers._size; i++) {
    int bundleIndex = reader.ReadType<uint32_t>(); // index of the bundle, important to be saved as pd2 skips the 67th bundle

    diesel::modern::SortMap<unsigned int, diesel::modern::Bundle::BundleEntry> header(reader, version);

    auto oldPosition = reader.GetPosition();

    reader.SetPosition(header._data._data + 4);

    Bundle::HeaderVectorType* header_vector = new Bundle::HeaderVectorType();

    for (int j = 0; j < header._data._size; j++) {
      auto key = reader.ReadType<uint32_t>();

      auto offset = reader.ReadType<uint32_t>();
      auto size = reader.ReadType<uint32_t>();

      header_vector->push_back(std::make_pair(key, Bundle::BundleEntry{ .offset = offset, .size = size }));
    }

    this->_headers.push_back(header_vector);
    this->_header_indices.insert(std::make_pair(header_vector, bundleIndex));

    reader.SetPosition(oldPosition);
  }

}
void Payday2StreamedBundleHeader::Write(Writer& writer, const diesel::DieselFormatsLoadingParameters& version)
{
  auto startPosition = writer.GetPosition();

  writer.WriteType<uint32_t>(0); // blob saver total size
  writer.WriteType<uint32_t>(_headers.size());
  writer.WriteType<uint32_t>(_headers.size()); // capacity

  //auto writeDataPosition = writer.GetPosition();
  //writer.WriteType(0);
  //writer.SetPosition(writeDataPosition);

  writer.WriteType<uint32_t>(16);

  writer.AddPosition(4); // reader.addposition(4) above, before main loop

  std::map<Bundle::HeaderVectorType*, uint64_t> headerOutPositions;
  for (int i = 0; i < _headers.size(); i++) {
    
    writer.WriteType<uint32_t>(_header_indices[_headers[i]]);

    uint64_t headerOutPos = 0;
    diesel::modern::SortMap<unsigned int, diesel::modern::Bundle::BundleEntry>::Write(writer, version, _headers[i]->size(), _headers[i]->size(), true, headerOutPos);
    headerOutPositions.insert(std::make_pair(_headers[i], headerOutPos));
  }

  for (int i = 0; i < _headers.size(); i++) {
    auto dataPos = writer.GetPosition();
    writer.SetPosition(headerOutPositions[_headers[i]]);
    if(_headers[i]->size() == 0) {
      writer.WriteType<uint32_t>(0);
    }
    else {
      writer.WriteType<uint32_t>(dataPos - 4);
    }
    writer.SetPosition(dataPos);

    for (auto& entry : (*_headers[i])) {
      writer.WriteType<uint32_t>(entry.first);
      writer.WriteType<uint32_t>(entry.second.offset);
      writer.WriteType<uint32_t>(entry.second.size);
    }
  }

  auto currentSize = writer.GetPosition();
  writer.SetPosition(startPosition);
  writer.WriteType<uint32_t>(currentSize);
  writer.SetPosition(currentSize);

  writer.WriteType<uint32_t>(0x94C51F19); // Bundle typeid
  writer.WriteType<uint32_t>(0); //writer.AddPosition(4);
}

void pd2_package_merger()
{
  std::filesystem::path assetsPath = R"(X:\SteamLibrary\steamapps\common\PAYDAY 2\assets_original)";

  Payday2StreamedBundleHeader streamedBundleHeader;
  {
    Reader allReader(assetsPath / "all_h.bundle");
    streamedBundleHeader.Read(allReader, diesel::EngineVersion::PAYDAY_2_LATEST);
    allReader.Close();
  }

  BundleDatabase bundleDatabase;
  {
    Reader bdbReader(assetsPath / "bundle_db.blb");
    bundleDatabase.Read(bdbReader, diesel::EngineVersion::PAYDAY_2_LATEST);
    bdbReader.Close();
  }

  std::set<unsigned int> luaFiles;
  for (auto& resource : bundleDatabase.GetLookup()) {
    if (resource.first._type == Idstring("lua")) {
      luaFiles.insert(resource.second);
    }
  }

  //std::set<ResourceID> uniqueResources;
  auto packages = open_packages(assetsPath.string().c_str());

  for (auto package : packages) {
    for (auto& resource : package->GetResources()) {
      //uniqueResources.insert(resource);
    }
  }

  std::set<unsigned int> processedDbKeys;

  std::map<unsigned int, EntryInfo> fileInfo;

  int current_streamed_bundle = 134;
  Writer outputFile(std::format("./all_{}.bundle", current_streamed_bundle));
  Bundle::HeaderVectorType* header_vec = new Bundle::HeaderVectorType();
  //streamedBundleHeader._headers.push_back(header_vec);
  //streamedBundleHeader._header_indices.insert({ header_vec, current_streamed_bundle });


  uint64_t outputFilePos = 0;
  for (auto package : packages) {
    std::set<unsigned int> packageKeys;
    for (auto& key : package->header) {
      packageKeys.insert(key.first);
    }

    for (auto& resource : packageKeys) {
      //unsigned int dbKey = bundleDatabase.GetDBKeyFromTypeAndName(resource.type, resource.name);

      unsigned int dbKey = resource;

      //if (luaFiles.contains(dbKey))
      //  __debugbreak();

      if (!processedDbKeys.contains(dbKey)) {
        processedDbKeys.insert(dbKey);
        Reader file;
        bool open = package->open(file, dbKey);


        if (open) {
          if (outputFilePos + file.GetFileSize() > 0xFFFFFFFF) {
            outputFilePos = 0;

            streamedBundleHeader._headers.push_back(header_vec);
            streamedBundleHeader._header_indices.insert({ header_vec, current_streamed_bundle });

            current_streamed_bundle++;
            header_vec = new Bundle::HeaderVectorType();
            outputFile.Close();
            outputFile = Writer(std::format("./all_{}.bundle", current_streamed_bundle));
          }

          /*if (luaFiles.contains(dbKey)) {
            auto fs = file.GetFileSize();
            char* buffer = new char[fs];
            file.ReadBytesToBuffer(buffer, fs);

            for (int i = 0; i < fs; i++) {
              int keyIndex = ((fs + i) * 7) % LuaDecryptionKeyLen;
              buffer[i] ^= (char)(LuaDecryptionKey[keyIndex] * (fs - i));
            }
            file = Reader(buffer, fs);
          }*/


          header_vec->push_back(std::make_pair(
            dbKey,
            Bundle::BundleEntry{ .offset = outputFilePos, .size = file.GetFileSize() }
          ));

          fileInfo.insert(std::make_pair(dbKey, EntryInfo{ .size = (unsigned int)file.GetFileSize(), .offset = (unsigned int)outputFilePos }));
          outputFilePos += file.GetFileSize();
          
          //fileInfo.insert(std::make_pair(dbKey, EntryInfo{ .size = (unsigned int)file.GetFileSize(), .offset = (unsigned int)outputFile.GetPosition() }));
          outputFile.WriteReader(file);
          file.Close();
        }
      }
    }
  }
  streamedBundleHeader._headers.push_back(header_vec);
  streamedBundleHeader._header_indices.insert({ header_vec, current_streamed_bundle });
  outputFile.Close();


  Writer allh("all_h.bundle");
  streamedBundleHeader.Write(allh, diesel::EngineVersion::PAYDAY_2_LATEST);
  allh.Close();

  /*for (auto& resource : uniqueResources) {
    auto dbKey = bundleDatabase.GetDBKeyFromTypeAndName(resource.type, resource.name);

    if (dbKey == -1)
      continue;

    Reader fileContents;
    if (open_package(packages, dbKey, fileContents)) {
      outputFile.WriteReader(fileContents);
    }
  }*/
  outputFile.Close();

  __debugbreak();
}

BundleDatabase openbdba(const std::filesystem::path& file, diesel::DieselFormatsLoadingParameters params)
{
  Reader r(file);
  r.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(params));
  BundleDatabase bdb;
  bdb.Read(r, params);
  r.Close();
  return bdb;
}

void load_hashlist()
{
  Reader hashlist("X:\\Projects\\DieselEngineExplorer\\hashlist.txt");
  diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
  hashlist.Close();
}
int main(int argc, char* argv[]) {
  load_hashlist();

  bc2_xb360_extract_test();
  return 0;
  std::filesystem::path NPUA30073path = R"(X:\Projects\rpcs3\dev_hdd0\game\NPUA30073\USRDIR\assets)";

  diesel::DieselFormatsLoadingParameters ps31 = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_THE_HEIST_V1, diesel::Renderer::PLAYSTATION3, diesel::FileSourcePlatform::SONY_PLAYSTATION_3);
  diesel::DieselFormatsLoadingParameters ps32 = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST, diesel::Renderer::PLAYSTATION3, diesel::FileSourcePlatform::SONY_PLAYSTATION_3);

  BundleDatabase NPEA00331 = openbdba(R"(X:\Projects\rpcs3\dev_hdd0\game\NPEA00331\USRDIR\assets\all.blb)", ps31);
  BundleDatabase NPUA30073 = openbdba(R"(X:\Projects\rpcs3\dev_hdd0\game\NPUA30073\USRDIR\assets\all.blb)", ps32);

  __debugbreak();

  return 0;
  /*{
    Reader hashlist("X:\\Projects\\DieselEngineExplorer\\hashlist.txt");
    diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
    hashlist.Close();
  }*/

  //bc2_xb360_extract_test();

  //Reader b("X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets\\bundle_db.blb");
  //BundleDatabase bd;
  //bd.Read(b, diesel::EngineVersion::PAYDAY_2_LATEST);

  //__debugbreak();
  //pd2_package_merger();

  //return 0;

  diesel::DieselFormatsLoadingParameters params = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_LATEST, diesel::Renderer::DIRECTX9, diesel::FileSourcePlatform::WINDOWS_32);

  //return 0;

  //std::filesystem::path optimised_assets_dir = "X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets_optimised";
  std::filesystem::path optimised_assets_dir = "X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets";

  /*
  {
    auto inpath = "X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets\\00b277d8d36bfd4c_h.bundle";
    auto outpath = "X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets_optimised\\00b277d8d36bfd4c_h.bundle";
    Reader pkgr(inpath);
    PackageBundle pkg(inpath, pkgr, params);

    Writer pkgw(outpath);
    pkg.Write(pkgw, params);
    pkgw.Close();

    Reader pkgr2(inpath);
    PackageBundle pkg2(inpath, pkgr2, params);
  }
  */

  for (std::filesystem::recursive_directory_iterator i("X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets_original"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h.bundle") == std::string::npos)
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;


      Reader reader1(i->path());
      PackageBundle package(i->path(), reader1, params);

      //package.header.clear();

      for (auto& resource : package.GetResources()) {
        //if(!package.stream_types.contains(resource.type))
          //package.stream_types.insert(resource.type);
      }

      Writer writer(optimised_assets_dir / i->path().filename());
      package.Write(writer, params);

      Writer contents(optimised_assets_dir / diesel::ReplaceInString(i->path().filename(), L"_h", L""));
      contents.Close();
    }
  }


  __debugbreak();
  return 0;

  //bc2_xb360_extract_test();


  return 0;

  /*Reader bdbr("X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets\\bundle_db.blb");
  BundleDatabase bdb;
  bdb.Read(bdbr, diesel::EngineVersion::PAYDAY_2_LATEST);

  std::map<std::pair<Idstring, Idstring>, int> fileSize;
  std::unordered_map<std::pair<Idstring, Idstring>, int> copies;
  for (std::filesystem::recursive_directory_iterator i("X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h.bundle") == std::string::npos)
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;


      Reader reader1(i->path());
      diesel::modern::blobtypes::PackageBundle package(i->path(), reader1, diesel::EngineVersion::PAYDAY_2_LATEST);

      for (auto& entry : package.GetResources()) {
        if (!copies.contains(std::make_pair(entry.name, entry.type))) {
          copies.insert(std::make_pair(std::make_pair(entry.name, entry.type), 1));
        }
        else {
          copies[std::make_pair(entry.name, entry.type)] += 1;
        }
        if (!fileSize.contains(std::make_pair(entry.name, entry.type))) {
          unsigned int dbKey = bdb.GetDBKeyFromTypeAndName(entry.type, entry.name);
          fileSize.insert(std::make_pair(std::make_pair(entry.name, entry.type), package.GetFileSize(dbKey)));
        }
      }

      //DumpPackage(i->path());
    }
  }

  std::vector<std::pair<std::pair<Idstring, Idstring>, int>> cursedlist;
  for(auto& entry : copies) {
    cursedlist.push_back(std::make_pair(entry.first, entry.second));
  }

  std::sort(cursedlist.begin(), cursedlist.end(), [](const auto& a, const auto& b) -> bool {
    if (a.second > b.second) return true;
    if (a.second == b.second) return a.first < b.first;
    return false;
    });

  std::ofstream outfile("fileusagepd2.csv");

  Hashlist* hashlistp = GetGlobalHashlist();

  outfile << "File Name,Occurences,File Size (In Bytes),Total Space Taken (In Bytes)\n";
  for (auto& entry : cursedlist) {
    long long file_size = -1;
    if (fileSize.contains(entry.first)) {
      file_size = fileSize[entry.first];
    }


    outfile << std::format("{}.{},{},{},{}\n", hashlistp->GetIdstringSource(entry.first.first), hashlistp->GetIdstringSource(entry.first.second), entry.second, file_size, entry.second * file_size);
  }
  outfile.close();

  return 0;*/
}
