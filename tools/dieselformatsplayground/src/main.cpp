#include "fileio/reader.h"
#include "diesel/lag/xml.h"
#include "diesel/lag/font.h"
#include "diesel/modern/bundle.h"
#include "diesel/font.h"
#include "diesel/modern/hashlist.h"
#include "diesel/objectdatabase.h"

#include <iostream>
#include <fstream>
#include <set>
#include <cassert>

void DumpBundle(diesel::modern::BundleDatabase& bundle_db) {
  std::vector<diesel::modern::ResourceID> resources;
  bundle_db.GetFileList(resources);

  std::string type;
  std::string name;
  for (auto& resource : resources) {
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.type, type);
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.name, name);
    std::cout << name << "." << type << std::endl;
  }
}

void DumpBundleFile(std::string path, diesel::modern::ModernEngineVersion version) {
  Reader reader(path);
  diesel::modern::BundleDatabase bdb(reader, version);
  DumpBundle(bdb);
}

void replaceinstr(std::string& in, std::string find, std::string replace) {
  std::string::size_type n = 0;
  while ((n = in.find(find, n)) != std::string::npos) {
    in.replace(n, find.size(), replace);
    n += replace.size();
  }
}

const char* LuaDecryptionKey = "asljfowk4_5u2333crj";
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


    for (int i = 0; i < fs; i++) {
      int keyIndex = ((fs + i) * 7) % LuaDecryptionKeyLen;
      buffer[i] ^= (char)(LuaDecryptionKey[keyIndex] * (fs - i));
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
  diesel::modern::BundleDatabase bdb(bdbreader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);

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
  bdb.Write(modifiedBdb, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);

}

void RemakePackageXml(const std::vector<diesel::modern::ResourceID>& package, std::string outFilePath, bool fonts_only) {
  //auto bundleDbPath = "X:\\Projects\\DieselEngineExplorer\\all.blb";
  //Reader bdbreader(bundleDbPath);
  //diesel::modern::BundleDatabase bdb(bdbreader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);

  std::vector<std::string> units;
  std::vector<std::string> massunits;
  std::vector<std::string> physic_effects;
  std::vector<std::string> fonts;
  std::vector<std::string> merged_fonts;
  std::vector<std::string> effects;
  std::vector<std::string> scenes;
  std::vector<std::string> soundbanks;
  std::vector<std::string> guis;

  std::vector<std::string> script_data;

  std::filesystem::path assets = "X:\\Projects\\RAIDMultiFileTransport\\normalassets";

  for (auto& entry : package) {
    std::string name;
    std::string type;
    if (!diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(entry.name, name))
      continue;
    if (!diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(entry.type, type))
      continue;

    if (!std::filesystem::exists(assets / (name + "." + type)))
      continue;

    if (fonts_only) {
      if (entry.type == diesel::modern::Idstring("font")) {
        fonts.push_back("<font name=\"" + name + "\" />");
        continue;
      }
      if (entry.type == diesel::modern::Idstring("merged_font")) {
        merged_fonts.push_back("<font name=\"" + name + "\" />");
        continue;
      }
      continue;
    }

    if (entry.type == diesel::modern::Idstring("texture"))
      continue;

    if (entry.type == diesel::modern::Idstring("scene")) {
      scenes.push_back("<scene name=\"" + name + "\" />");
      continue;
    }
    if (entry.type == diesel::modern::Idstring("unit")) {
      units.push_back("<unit name=\"" + name + "\" />");
      continue;
    }
    if (entry.type == diesel::modern::Idstring("bnk")) {
      soundbanks.push_back("<bnk name=\"" + name + "\" />");
      continue;
    }
    if (entry.type == diesel::modern::Idstring("massunit")) {
      massunits.push_back("<massunit name=\"" + name + "\" />");
      continue;
    }
    if (entry.type == diesel::modern::Idstring("gui")) {
      guis.push_back("<gui name=\"" + name + "\" />");
      continue;
    }
    if (entry.type == diesel::modern::Idstring("effect")) {
      effects.push_back("<effect name=\"" + name + "\" />");
      continue;
    }
    if (entry.type == diesel::modern::Idstring("physic_effect")) {
      physic_effects.push_back("<physic_effect name=\"" + name + "\" />");
      continue;
    }

    // from bool custom_type(dsl::idstring name)
    // shaders? maybe.
    // strings
    // decals
    // animation_def
    // animation_state_machine
    // post_processor
    // interaction
    // cgb
    // lua

    {
#define SCRIPT_DATA_ENTRY_CHECK(typea) if(entry.type == diesel::modern::Idstring(#typea)) {script_data.push_back("<" #typea " name=\"" + name + "\" />"); continue;}
      // some from bool xml_type(dsl::idstring name)
      SCRIPT_DATA_ENTRY_CHECK(continent)
      SCRIPT_DATA_ENTRY_CHECK(world)
      SCRIPT_DATA_ENTRY_CHECK(environment)
      SCRIPT_DATA_ENTRY_CHECK(mission)
      SCRIPT_DATA_ENTRY_CHECK(sequence_manager)
      SCRIPT_DATA_ENTRY_CHECK(achievment)
      SCRIPT_DATA_ENTRY_CHECK(menu)
      SCRIPT_DATA_ENTRY_CHECK(objective)
      SCRIPT_DATA_ENTRY_CHECK(state)
      SCRIPT_DATA_ENTRY_CHECK(credits)
      SCRIPT_DATA_ENTRY_CHECK(hint)
      SCRIPT_DATA_ENTRY_CHECK(continents)
      SCRIPT_DATA_ENTRY_CHECK(dialog)
      SCRIPT_DATA_ENTRY_CHECK(dialog_index)
      //SCRIPT_DATA_ENTRY_CHECK(merged_font)
      SCRIPT_DATA_ENTRY_CHECK(material_config)
      SCRIPT_DATA_ENTRY_CHECK(atom_batcher_settings)
      SCRIPT_DATA_ENTRY_CHECK(cameras)
      SCRIPT_DATA_ENTRY_CHECK(physics_settings)
      SCRIPT_DATA_ENTRY_CHECK(diesel_layers)
      SCRIPT_DATA_ENTRY_CHECK(texture_channels)
      SCRIPT_DATA_ENTRY_CHECK(light_intensities)
      SCRIPT_DATA_ENTRY_CHECK(network_settings)
      SCRIPT_DATA_ENTRY_CHECK(camera_shakes)
      SCRIPT_DATA_ENTRY_CHECK(scenes)
      SCRIPT_DATA_ENTRY_CHECK(render_template_database)

      SCRIPT_DATA_ENTRY_CHECK(nav_data)
      SCRIPT_DATA_ENTRY_CHECK(cover_data)
      SCRIPT_DATA_ENTRY_CHECK(strings)
      SCRIPT_DATA_ENTRY_CHECK(world_sounds)
      SCRIPT_DATA_ENTRY_CHECK(world_cameras)
      SCRIPT_DATA_ENTRY_CHECK(controller_settings)
      SCRIPT_DATA_ENTRY_CHECK(texture_channels)
    }

  }

  if (!std::filesystem::exists(std::filesystem::path(outFilePath).parent_path()))
    std::filesystem::create_directories(std::filesystem::path(outFilePath).parent_path());

  std::ofstream outFile(outFilePath);

  outFile << "<package>\n";

  outFile << "\t<streaming win32=\"all\" xb1=\"all\" ps4=\"all\"/>\n"; // formats supports "{single_format}", "{format1} {format2}..." and "all"

#define MONO_PACKAGE_HELPER(arrayName, xmlTag) \
outFile << "\t<" #xmlTag ">\n"; \
for(auto& entry : arrayName) outFile << "\t\t" << entry << '\n'; \
outFile << "\t</" #xmlTag ">\n";

  MONO_PACKAGE_HELPER(units, units);
  MONO_PACKAGE_HELPER(massunits, massunits);
  MONO_PACKAGE_HELPER(physic_effects, physic_effects);
  //MONO_PACKAGE_HELPER(fonts, fonts);

  outFile << "\t<fonts>\n";
  for (auto& font : fonts) { outFile << "\t\t" << font << '\n'; }
  for (auto& font : merged_fonts) { outFile << "\t\t" << font << '\n'; }
  outFile << "\t</fonts>\n";
  MONO_PACKAGE_HELPER(effects, effects);
  MONO_PACKAGE_HELPER(scenes, scenes);
  MONO_PACKAGE_HELPER(soundbanks, soundbanks);
  MONO_PACKAGE_HELPER(guis, guis);
  MONO_PACKAGE_HELPER(script_data, script_data);



  outFile << "</package>\n";

  outFile.close();
}

void DumpPackage(const std::filesystem::path& path) {
  Reader reader1(path);

  Reader reader2;
  reader1.ReadCompressed(reader2);
  reader1.Close();

  diesel::modern::blobtypes::PackageBundle package(path, reader2, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);

  std::string pkgName = "";

  auto packageName = std::stoull(path.filename().replace_extension("").string().substr(0, 16), 0, 16);

  if (!diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(diesel::modern::Idstring(_byteswap_uint64(packageName)), pkgName)) {
    std::cout << "Couldn't find name for " << path.string() << std::endl;
    return;
  }

  RemakePackageXml(package.GetResources(), "./packages/" + pkgName + ".package", false);
}

auto assetsPath = std::filesystem::path("X:\\Projects\\RAIDMultiFileTransport\\normalassets");
auto assetsPathStr = assetsPath.string();
void CopyMultiFileTransport() {
  std::filesystem::path bundleDbPath = "X:\\Projects\\DieselEngineExplorer\\bundle_db.blb";
  //auto bundleDbPath = "X:\\Projects\\DieselEngineExplorer\\bundle_db.blb";
  //auto bundleDbPath = "X:\\SteamLibrary\\steamapps\\common\\PAYDAY The Heist\\assets\\all.blb";
  Reader bdbreader(bundleDbPath);
  diesel::modern::BundleDatabase bdba(bdbreader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);

  //diesel::modern::BundleDatabase bdb(bdbreader, diesel::modern::ModernEngineVersion::PAYDAY_THE_HEIST_V1);


  std::filesystem::path multifileOut = "X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assets";
  diesel::modern::Idstring lua = diesel::modern::Idstring("lua");

  std::ofstream logOut("multifiletransportlog.txt");

  for (auto& entry : bdba.GetLookup()) {
    auto dbKey = entry.second;
    auto allFolder = std::string("all") + std::to_string(dbKey % 512);

    std::string type;
    std::string name;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(entry.first._name, name);
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(entry.first._type, type);

    if (type == "movie")
      type = "bk2";
    if (type == "texture")
      type = "dds";

    auto sourcePath = (assetsPath / name).string();
    sourcePath += "." + type;

    auto destPath = multifileOut / allFolder / std::to_string(dbKey);

    auto isLua = entry.first._type == lua;

    if (!std::filesystem::exists(destPath.parent_path()))
      std::filesystem::create_directories(destPath.parent_path());

    if (!std::filesystem::exists(sourcePath)) {
      logOut << "Non existant source: " << sourcePath << "\n";
      //std::cout << "Non existant source: " << sourcePath << "\n";
      continue;
    }
    if (std::filesystem::exists(destPath))
      continue;


    copyfile(sourcePath, destPath.string(), isLua);
  }

  logOut.close();
}

int main() {
  /*Reader hashlist("X:\\Projects\\DieselEngineExplorer\\hashlist.txt");
  diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
  hashlist.Close();*/

  /*Reader bdbReader("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assets\\bundle_db.blb");
  diesel::modern::BundleDatabase bdb(bdbReader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);
  bdbReader.Close();

  Reader editorReader(std::string("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assets\\") + diesel::modern::Idstring("core/packages/editor").hex() + "_h.bundle");
  diesel::modern::blobtypes::PackageBundle editor("", editorReader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);
  editorReader.Close();

  for (auto& resource : editor.GetResources()) {
    auto key = bdb.GetDBKeyFromTypeAndName(resource.type, resource.name);

    std::string type;
    std::string name;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.type, type);
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.name, name);

    std::cout << name << "." << type << "\n";

  }*/

  //Reader raidfontReader("X:\\Projects\\RAIDMultiFileTransport\\normalassets\\core\\fonts\\nice_editor_font.font");
  //diesel::AngelCodeFont raidfont(raidfontReader, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);
  //Reader pd2fontReader("X:\\Projects\\DieselEngineExplorer\\test_files\\fonts\\pd2\\nice_editor_font.font");
  //diesel::AngelCodeFont pd2font(pd2fontReader, diesel::EngineVersion::PAYDAY_2_LATEST);
  //Reader lagAbcFont("X:\\Projects\\DieselEngineExplorer\\test_files\\fonts\\lag\\bcfont_14.abc");
  //diesel::lag::FontMakerFont lagAbcFontLoaded(lagAbcFont, diesel::EngineVersion::LEAD_AND_GOLD);

  //Reader blbFont("X:\\Projects\\DieselEngineExplorer\\test_files\\fonts\\lag\\bcfont_11.blb");
  //diesel::AngelCodeFont fnt(blbFont, diesel::EngineVersion::LEAD_AND_GOLD);

  return 0;

  /*Reader reader1("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assetsa\\stream_default_0_h.bundle");

  Reader reader2;
  reader1.ReadCompressed(reader2);
  reader1.Close();
  auto siz = reader2.GetFileSize();
  char* buf = new char[siz];
  reader2.ReadBytesToBuffer(buf, siz);
  Writer writer("./stream_default_0_h_uncomp.bundle");
  writer.WriteBytes(buf, siz);
  delete[] buf;
  reader2.SetPosition(0);*/

  /*diesel::modern::Bundle bundle("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assets\\", "", diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);*/


  /*Writer w8("./player_uk_gear_default_001_nm.dds");
  Reader r9;
  bundle.open(r9, 23543);
  w8.WriteReader(r9);
  w8.Close();*/

  /*Reader testReader1;
  Reader testReader2;
  std::cout << "2170: " << bdb.GetLookupInformationFromDBKey(2170)._name << "." << bdb.GetLookupInformationFromDBKey(2170)._type << "\n";
  bundle.open(testReader1, 2170);
  std::cout << "2191: " << bdb.GetLookupInformationFromDBKey(2191)._name << "." << bdb.GetLookupInformationFromDBKey(2191)._type << "\n";
  bundle.open(testReader2, 2191);

  Writer w1("./vehicle_explosion.effect");
  w1.WriteReader(testReader1);
  w1.Close();
  Writer w2("./props_sc250_junkers_destructable.object");
  w2.WriteReader(testReader2);
  w2.Close();*/

  /*Reader testReader3;
  bundle.open(testReader3, 43);
  Writer w3("./dome_occ_test.texture");
  w3.WriteReader(testReader3);
  w3.Close();*/

  /*diesel::modern::blobtypes::PackageBundle package("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assetsa\\ff03667ec101addd_h.bundle", reader2, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);
  Reader reader3;
  package.open(reader3, 1925);

  auto siz = reader3.GetFileSize();
  char* buf = new char[siz];
  reader3.ReadBytesToBuffer(buf, siz);
  Writer out("./plane_black_temp.unit");
  out.WriteBytes(buf, siz);
  delete[] buf;*/
  
  /*Reader bdbreader("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assetsb\\all.blb");
  diesel::modern::BundleDatabase bdb(bdbreader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);*/

  /*std::set<unsigned int> keys;

  for (std::filesystem::recursive_directory_iterator i("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assets"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      try {
        if (i->path().extension() != "")
          continue;

        unsigned int db_key = (unsigned int)std::stoul(i->path().filename());

        keys.insert(db_key);

      }
      catch (std::exception& e) {
        continue;
      }
    }
  }

  std::set<unsigned int> bdb_keys;

  for (auto& entry : bdb.GetLookup()) {
    bdb_keys.insert(entry.second);
  }

  std::set<unsigned int> missing_keys;

  std::set_difference(bdb_keys.begin(), bdb_keys.end(), keys.begin(), keys.end(), std::inserter(missing_keys, missing_keys.end()));

  for (auto& missing_key : missing_keys) {
    std::cout << "Missing key: " << missing_key << "\n";
  }
  std::cout << "Missing " << missing_keys.size() << " keys!\n";*/
  

  /*for (std::filesystem::recursive_directory_iterator i(assetsPath), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      auto fullPath = i->path().string();
      replaceinstr(fullPath, "\\", "/");

      auto relativePath = std::filesystem::path(fullPath.substr(assetsPathStr.size(), fullPath.size() - assetsPathStr.size()));

      relativePath.replace_extension("");
      diesel::modern::GetGlobalHashlist()->AddSourceToHashlist(relativePath.string());
    }
  }*/

  //CopyMultiFileTransport();
  //AddDummyPackageFileToRaid();

  /*auto bundleDbPatha = "X:\\Projects\\DieselEngineExplorer\\bundle_db.blb";
  Reader bdbreader(bundleDbPatha);
  diesel::modern::BundleDatabase bdb(bdbreader, diesel::modern::ModernEngineVersion::RAID_WORLD_WAR_II_LATEST);
  std::vector<diesel::modern::ResourceID> resources;
  for (auto& entry : bdb.GetLookup()) {
    resources.push_back(diesel::modern::ResourceID{ .type = entry.first._type, .name = entry.first._name });
  }
  RemakePackageXml(resources, "./monopackage.xml", false);
  RemakePackageXml(resources, "./monopackage_fonts.xml", true);*/

  //AddDummyPackageFileToRaid();
  //return 0;

  /*for (std::filesystem::recursive_directory_iterator i("X:\\SteamLibrary\\steamapps\\common\\RAID World War II\\assetsa"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h.bundle") == std::string::npos)
        continue;
      if (i->path().string().find("stream") != std::string::npos)
        continue;

      DumpPackage(i->path());
    }
  }*/
  //RemakePackageXmls();

  //AddDummyPackageFileToRaid();

  //Reader model("X:\\Projects\\RAIDMultiFileTransport\\normalassets\\units\\vanilla\\props\\prop_bucket_metal\\prop_bucket_metal.model");
  //Reader model("X:\\Projects\\RAIDMultiFileTransport\\normalassets\\core\\units\\run_sequence_dummy\\run_sequence_dummy.model");
  //diesel::ObjectDatabase odb(model, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

  //Reader xmb("X:\\SteamLibrary\\steamapps\\common\\Lead and Gold Gangs of the Wild West\\bundles\\quick\\compiled\\win32\\data\\settings\\graphic_quality.xmb");
  /*Reader xmb("X:\\SteamLibrary\\steamapps\\common\\Bionic Commando\\bundles\\quick\\compiled\\win32\\data\\settings\\unit_editor_macros.xmb");
  diesel::lag::XMLDocument xml;
  xml.ReadFromBinary(xmb, diesel::EngineVersion::BIONIC_COMMANDO);

  std::cout << xml.DumpRootToString() << "\n";*/



  /*Reader banditsEncrypted("C:\\Program Files (x86)\\PanVision\\BANDITS - Phoenix Rising\\data\\menu\\Main\\Main.dsf.enc");
  Reader banditsEncrypted("C:\\Program Files (x86)\\PanVision\\BANDITS - Phoenix Rising\\data\\maps\\mission01\\map.dsf.enc");
  Reader unencrypted;
  banditsEncrypted.ReadBANDITSEncryptedFile(unencrypted);
  auto siz = unencrypted.GetFileSize();
  char* encrypted_data = new char[siz+1] {};
  encrypted_data[siz] = '\x00';
  unencrypted.ReadBytesToBuffer(encrypted_data, siz);

  std::cout << encrypted_data << std::endl;

  delete[] encrypted_data;*/
  return 0;
}