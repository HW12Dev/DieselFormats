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

#include <iostream>
#include <fstream>
#include <set>
#include <cassert>
#include <functional>
#include <utility>
#include <unordered_set>

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

  diesel::modern::blobtypes::PackageBundle package(path, reader1, diesel::EngineVersion::PAYDAY_2_LATEST);

  std::string pkgName = "";

  auto packageName = std::stoull(path.filename().replace_extension("").string().substr(0, 16), 0, 16);

  if (!diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(diesel::modern::Idstring(_byteswap_uint64(packageName)), pkgName)) {
    std::cout << "Couldn't find name for " << path.string() << std::endl;
    //return;
  }

  RemakePackageXml(package.GetResources(), "./pd2packages/" + pkgName + ".package", false);
}

#pragma endregion


namespace std {
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
}


int main() {
  //Reader hashlist("X:\\Projects\\DieselEngineExplorer\\hashlist.txt");
  //diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
  //hashlist.Close();

  auto testsd = [](std::string path, diesel::EngineVersion ver) {
    Reader r(path);
    diesel::modern::ScriptData sd;
    sd.Read(r, ver);

    std::cout << diesel::modern::ScriptData::DumpScriptDataToGenericXml(sd) << std::endl;
    };

  auto tested = [](std::string path, diesel::EngineVersion ver) {
    Reader r(path);
    diesel::modern::EngineData sd;
    sd.Read(r, ver);

    std::cout << diesel::modern::EngineData::DumpReferenceToXml(sd.GetRoot()) << std::endl;
    };

  tested("X:/Projects/DieselEngineExplorer/test_files/enginedata/pdthlatest/render_config.render_config", diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST);
  tested("X:/Projects/DieselEngineExplorer/test_files/enginedata/pd2/render_config.render_config", diesel::EngineVersion::PAYDAY_2_LATEST);
  tested("X:/Projects/DieselEngineExplorer/test_files/enginedata/raid/render_config.render_config", diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

  testsd("X:/Projects/DieselEngineExplorer/test_files/scriptdata/pdthlatest/bank_world.world", diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST);
  testsd("X:/Projects/DieselEngineExplorer/test_files/scriptdata/pd2/firestarter_day3_world.world", diesel::EngineVersion::PAYDAY_2_LATEST);
  testsd("X:/Projects/DieselEngineExplorer/test_files/scriptdata/raid/forest_gumpy_world.world", diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

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