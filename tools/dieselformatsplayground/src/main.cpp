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
#include "diesel/objectdatabase_shaders.h"
#include "diesel/oil.h"
#include "diesel/random.h"
#include "diesel/modern/enginedata.h"
#include "diesel/modern/scriptdata.h"
#include "diesel/modern/strings.h"
#include "diesel/modern/banksinfo.h"
#include "diesel/modern/savegame.h"
#include "diesel/modern/misc.h"
#include "diesel/modern/massunit.h"
#include "diesel/animation.h"

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
  //std::filesystem::path inputDirectory = R"(X:\payday2\76791\assets)";
  std::filesystem::path inputDirectory = R"(X:\SteamLibrary\steamapps\common\PAYDAY 2\assets)";
  //std::filesystem::path outputDirectory = R"(X:\payday2\76791\assetsunpack)";
  std::filesystem::path outputDirectory = R"(X:\SteamLibrary\steamapps\common\PAYDAY 2\linearfiletransport)";
  //std::filesystem::path inputDirectory = R"(./assets)";
  //std::filesystem::path outputDirectory = R"(./assetsunpack)";

  //diesel::DieselFormatsLoadingParameters loadingParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::BIONIC_COMMANDO_REARMED2, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::MICROSOFT_XBOX_360);
  //diesel::DieselFormatsLoadingParameters loadingParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::SONY_PLAYSTATION_3);
  diesel::DieselFormatsLoadingParameters loadingParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_LATEST, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::WINDOWS_32);

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

    //std::string outputFileName = hashlist->GetIdstringSource(entry.first._name);
    //
    //int entryProperties = entry.first._properties;
    //if (entryProperties != 0) {
    //  for (auto& prop : properties) {
    //    if ((entryProperties & prop.first) != 0) {
    //      outputFileName += "." + prop.second;
    //    }
    //  }
    //}
    //
    //std::filesystem::path entryOutPath = outputDirectory / (outputFileName + "." + hashlist->GetIdstringSource(entry.first._type));

    std::filesystem::path entryOutPath = outputDirectory / (std::string("all") + (std::to_string(entry.second % 512) + "/" + std::to_string(entry.second)));

    if(!std::filesystem::exists(entryOutPath.parent_path()))
      std::filesystem::create_directories(entryOutPath.parent_path());

    try {
      Writer outWriter(entryOutPath);
      //outWriter.WriteReader(entryContents);
      outWriter.WriteReaderToCompressedDataStore(entryContents);
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

std::vector<PackageBundle*> open_packages(const char* dir, diesel::DieselFormatsLoadingParameters version)
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
      if (i->path().string().find("stream") != std::string::npos)
        continue;


      Reader reader1(i->path());
      PackageBundle* package = new PackageBundle(i->path(), reader1, version);


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
  Payday2StreamedBundleHeader(const std::filesystem::path& rootdir) : rootdir(rootdir) {};

public:
  void Read(Reader& reader, const diesel::DieselFormatsLoadingParameters& version);
  void Write(Writer& writer, const diesel::DieselFormatsLoadingParameters& version);

  bool open(Reader& outReader, unsigned int dbKey);

  bool open(Reader& outReader, unsigned int streamedIndex, const diesel::modern::Bundle::BundleEntry& entry);

private:
  Reader get_reader_for_bundle(unsigned int index)
  {
    if (headers.contains(index))
      return headers[index];

    std::filesystem::path bndl = rootdir / ("all_" + std::to_string(index) + ".bundle");

    if (!std::filesystem::exists(bndl))
      return Reader();

    Reader newreader(bndl);

    headers[index] = newreader;

    return newreader;
  }

public:
  std::vector<Bundle::HeaderVectorType*> _headers;
  std::map<Bundle::HeaderVectorType*, int> _header_indices;

private:
  std::filesystem::path rootdir;
  std::map<int, Reader> headers;
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


bool Payday2StreamedBundleHeader::open(Reader& outReader, unsigned int dbKey)
{
  return false;
}


bool Payday2StreamedBundleHeader::open(Reader& outReader, unsigned int streamedIndex, const diesel::modern::Bundle::BundleEntry& entry)
{
  Reader filereader = get_reader_for_bundle(streamedIndex);
  if (!filereader.Valid())
    return false;

  filereader.SetPosition(entry.offset);
  filereader.SetReplacementSize(entry.size);

  outReader = filereader;

  return true;
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
  if (std::filesystem::exists("hashlist.txt")) {
    Reader hashlist("hashlist.txt");
    diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
    hashlist.Close();
  }
  else if (std::filesystem::exists("hashlist")) {
    Reader hashlist("hashlist");
    diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
    hashlist.Close();
  }
}


diesel::objectdatabase::ObjectDatabase* load_objdb(std::filesystem::path path, diesel::DieselFormatsLoadingParameters params)
{
  Reader r(path);
  diesel::objectdatabase::ObjectDatabase* odb = new diesel::objectdatabase::ObjectDatabase(r, params);
  return odb;
}



int fakemain(int argc, char* argv[]);

int main(int argc, char* argv[])
{
#define ERRORHANDLE 0
#if ERRORHANDLE
  try {
    return fakemain(argc, argv);
  }
  catch (std::exception& e) {
    printf("Error while running DieselFormatsPlayground: %s\n", e.what());
    system("pause");
    return 1;
  }
#else
  return fakemain(argc, argv);
#endif
}

std::string get_indent_(int i)
{
  std::string o;
  while (i > 0) {
    o += "  ";
    i--;
  }
  return o;
}

void printd3dstatevar(const diesel::objectdatabase::typeidclasses::shaders::d3d::D3DStateVariable& var)
{
  switch (var.type) {
  case shaders::d3d::D3DStateVariableType::CONSTANT:
    printf("CONSTANT: %i", (signed)var.constant);
    break;
  case shaders::d3d::D3DStateVariableType::REFERENCE:
    printf("REFERENCE: \"%s\"", GetGlobalHashlist()->GetIdstringSource(var.reference).c_str());
    break;
  default:
    printf("UNKNOWN (%u)", var.type);
    break;
  }
}
void printd3dstateblock(const diesel::objectdatabase::typeidclasses::shaders::d3d::D3DStateBlock& block, int indent)
{
  for (auto& state : block.states) {
    printf("%s", get_indent_(indent).c_str());
    printf("%u: ", state.first);
    printd3dstatevar(state.second);
    printf("\n");
  }
}
void shaderinfodump(int argc, char* argv[])
{
  argc = 2;
  const char* argv_[2];
  argv = const_cast<char**>(argv_);
  
  argv_[0] = "";
  //argv_[1] = R"(X:\Projects\DieselEngineExplorer\test_files\shaders\pd2win32\base.d3d11.shaders)";
  argv_[1] = R"(X:\SteamLibrary\steamapps\common\Bionic Commando\bundles\quick\db\model\f\fi_water_a_80x80.diesel)";
  if (argc != 2) {
    printf("DieselFormatsPlayground shaderdb.shaders\n");
    return;
  }

  load_hashlist();

  using namespace diesel::objectdatabase;

  auto params = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::BIONIC_COMMANDO, diesel::Renderer::DIRECTX9, diesel::FileSourcePlatform::WINDOWS_32);

  diesel::objectdatabase::ObjectDatabase* odb = load_objdb(argv[1], params);

  for (PersistentObject* obj : odb->GetObjects()) {
    if (obj->type_id() == diesel::objectdatabase::typeids::D3DShaderLibraryData) {
      D3DShaderLibraryData* shaderlib = (D3DShaderLibraryData*)obj;

      for (auto& shader : shaderlib->_shaders) {
        printf("%s\n", GetGlobalHashlist()->GetIdstringSource(shader.first).c_str());
        printf("  Layers:\n");
        for (auto& layer : shader.second->_layers) {
          printf("    %s\n", GetGlobalHashlist()->GetIdstringSource(layer.first).c_str());
          printd3dstateblock(layer.second->_render_states, 3);

          printf("    Sampler State Blocks:\n");
          for (auto& stateblock : layer.second->_dx11_sampler_state_blocks) {
            printf("      %s\n", GetGlobalHashlist()->GetIdstringSource(stateblock.first).c_str());
            printd3dstateblock(stateblock.second, 4);
          }
        }


        printf("\n");
      }

      break;
    }
  }
}


unsigned int GetWwiseHash(const char* str)
{
  unsigned int hash = 2166136261;
  for (int i = 0; i < strlen(str); i++) {
    hash = str[i] ^ (16777619 * hash);
  }
  return hash;
}
int fakemain(int argc, char* argv[]) {
  //load_hashlist();

  /*BundleDatabase bd;
  Reader bdr(R"(X:\SteamLibrary\steamapps\common\RAID World War II\assets\bundle_db.blb)");
  bd.Read(bdr, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

  int key = bd.GetDBKeyFromTypeAndName(Idstring("movie"), Idstring("movies/vanilla/debrief_failure/f_11_sings_v007"));

  Bundle bndl(R"(X:\SteamLibrary\steamapps\common\RAID World War II\assets)", "", diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

  auto packages = open_packages(R"(X:\SteamLibrary\steamapps\common\RAID World War II\assets)", diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

  Reader movie;
  if (!bndl.open(movie, key)) {
    for (auto package : packages) {
      if (package->open(movie, key)) {
        break;
      }
    }
  }*/

  //Payday2StreamedBundleHeader pd2streamed(R"(X:\SteamLibrary\steamapps\common\PAYDAY 2\assets)");
  //Reader hdrr(R"(X:\SteamLibrary\steamapps\common\PAYDAY 2\assets\all_h.bundle)");
  //pd2streamed.Read(hdrr, diesel::EngineVersion::PAYDAY_2_LATEST);
  
  //std::cout << GetWwiseHash("weapon_cool_custom_bank") << std::endl;
  //std::cout << GetWwiseHash("soundbanks/weapon_cool_custom_bank") << std::endl;
  //std::cout << Idstring("soundbanks/weapon_cool_custom_bank").operator unsigned long long() << std::endl;


  /*Reader r(R"(X:\Projects\DieselEngineExplorer\pd2_existing_banks.banksinfo)");
  BanksInfo banksinfo;
  banksinfo.Read(r, diesel::EngineVersion::PAYDAY_2_LATEST);
  r.Close();

  Writer w(R"(X:\Projects\DieselEngineExplorer\pd2_existing_banks_rewritten.banksinfo)");
  banksinfo.Write(w, diesel::EngineVersion::PAYDAY_2_LATEST);
  w.Close();*/

  /*for (std::filesystem::recursive_directory_iterator i(R"(X:\SteamLibrary\steamapps\common\PAYDAY 2\assets)"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h") == std::string::npos)
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;
      if (i->path().string().find("stream") != std::string::npos)
        continue;


      DumpPackage(i->path());
    }
  }*/


  //Reader shdrs(R"(X:\SteamLibrary\steamapps\common\PAYDAY 2\shaders.shaders)");

  //diesel::objectdatabase::ObjectDatabase odb(shdrs, diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_LATEST, diesel::Renderer::DIRECTX11, diesel::FileSourcePlatform::WINDOWS_32));

  /*Reader anim(R"(X:\Projects\DieselEngineExplorer\test_files\animations\pd2_cloaker_jump_attack.animation)");

  diesel::Animation a;
  a.Read(anim, diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_LATEST));


  return 0;
  //Reader savegame(R"(C:\Users\hayde\AppData\Local\PAYDAY 2\saves\76561198392993837\save098.sav)");
  //Reader savegame(R"(C:\Users\hayde\AppData\Local\PAYDAY 2 88892\saves\76561198392993837\save069.sav)");
  //Reader savegame(R"(C:\Users\hayde\AppData\Local\PAYDAY\saves\76561198392993837\save099.sav)");
  Reader savegame(R"(D:\Program Files (x86)\Steam\userdata\432728109\21670\remote\save4.sav)");
  //Reader savegame(R"(D:\Program Files (x86)\Steam\userdata\432728109\414740\remote\save000.sav)");

  const char* key = "SAVE FILE KEY";
  size_t key_len = strlen(key);

  Reader decrypted;
  //savegame.ReadWithXORKeyTransformInPlace(decrypted, key, key_len);
  //savegame.ReadWithXORKey(decrypted, key, key_len);
  //decrypted = savegame;


  SaveGame save;
  save.Read(savegame, diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::BIONIC_COMMANDO));

  __debugbreak();*/

  return 0;
}
