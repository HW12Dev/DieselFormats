#include "fileio/reader.h"
#include "fileio/zlibcompression.h"
#include "diesel/graw/dieselscript.h"
#include "diesel/lag/xml.h"
#include "diesel/modern/bundle.h"
#include "diesel/font.h"
#include "diesel/modern/hashlist.h"
#include "diesel/objectdatabase.h"
#include "diesel/oil.h"

#include <iostream>
#include <fstream>
#include <set>
#include <cassert>

#pragma region Testing Functions

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

void DumpBundleFile(std::string path, diesel::EngineVersion version) {
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
  diesel::modern::BundleDatabase bdb(bdbreader, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

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
  reader1.ReadCompressedDataStore(reader2);
  reader1.Close();

  diesel::modern::blobtypes::PackageBundle package(path, reader2, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

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
  diesel::modern::BundleDatabase bdba(bdbreader, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

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

#pragma endregion

int main() {
  return 0;
  //Reader hashlist("X:\\Projects\\DieselEngineExplorer\\hashlist.txt");
  //diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
  //hashlist.Close();

  /*Reader idstring_lookup("X:\\Projects\\DieselEngineExplorer\\build\\windows\\x64\\release\\pdthps3\\idstring_lookup.idstring_lookup");
  diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(idstring_lookup);
  idstring_lookup.Close();

  Writer hash2("./savedhashlist.txt");
  diesel::modern::GetGlobalHashlist()->DumpHashlistToFile(hash2);
  hash2.Close();*/

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

  //Reader shaders("X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\raid\\base.shaders");
  //diesel::ObjectDatabase shadersObjdb(shaders, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST, diesel::Renderer::DIRECTX11);

  //Reader pd2oglshadersrr("X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\pd2linux\\default_shaders.d3d10.shaders");
  //diesel::ObjectDatabase pd2oglshadersa(pd2oglshadersrr, diesel::EngineVersion::PAYDAY_2_LINUX_LATEST, diesel::Renderer::DIRECTX10);
  /*Reader pd2oglshadersr("X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\pd2linux\\base.ogl.shaders");
  diesel::ObjectDatabase pd2oglshaders(pd2oglshadersr, diesel::EngineVersion::PAYDAY_2_LINUX_LATEST, diesel::Renderer::OPENGL);
  pd2oglshadersr.Close();

  diesel::typeidclasses::D3DShaderLibraryData* shaderLibrary;

  for (auto object : pd2oglshaders.GetObjects()) {
    if (object->type_id() == diesel::typeids::D3DShaderLibraryData) {
      shaderLibrary = (diesel::typeidclasses::D3DShaderLibraryData*)object;
      break;
    }
  }

  for (auto& shader : shaderLibrary->_shaders) {
    //std::string name;
    //diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(shader.first, name);
    //std::cout << name << std::endl;

    for (auto& layer : shader.second->_layers) {
      __debugbreak();
    }
  }

  for (auto object : pd2oglshaders.GetObjects()) {
    if (object->type_id() != diesel::typeids::D3DShaderData)
      continue;

    __debugbreak();
  }

  for (auto object : pd2oglshaders.GetObjects()) {
    if (object->type_id() != diesel::typeids::D3DShaderPassData)
      continue;

    __debugbreak();
  }*/

  //Reader lagshadersr("X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\lag\\d3d10_base_win32.diesel");
  //diesel::ObjectDatabase lagshaders(lagshadersr, diesel::EngineVersion::LEAD_AND_GOLD, diesel::Renderer::DIRECTX10);
  
  //lagshadersr.Close();
  
  //Reader ps3terminatorshaders("D:\\Program Files (x86)\\Evolved Games\\Terminator Salvation\\bundles\\quick\\core\\shaders\\ps3\\default_shaders.diesel");
  //diesel::objectdatabase::ObjectDatabase ps3terminator(ps3terminatorshaders, diesel::EngineVersion::TERMINATOR_SALVATION, diesel::FileSourcePlatform::WINDOWS_32, diesel::Renderer::PLAYSTATION3);
  //Reader xb360terminatorshaders("D:\\Program Files (x86)\\Evolved Games\\Terminator Salvation\\bundles\\quick\\core\\shaders\\x360\\default_shaders.diesel");
  //diesel::ObjectDatabase xb360terminator(xb360terminatorshaders, diesel::EngineVersion::TERMINATOR_SALVATION, diesel::Renderer::DIRECTX9);
  //Reader winterminatorshaders("D:\\Program Files (x86)\\Evolved Games\\Terminator Salvation\\bundles\\quick\\core\\shaders\\win32_dx10\\default_shaders.diesel");
  //diesel::ObjectDatabase winterminator(winterminatorshaders, diesel::EngineVersion::TERMINATOR_SALVATION, diesel::Renderer::DIRECTX10);

  //Reader oilreader("X:\\Projects\\DieselEngineExplorer\\test_files\\oil\\raid\\anim_cube.model");
  //diesel::oil::OIL oil(oilreader, diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST);

  //diesel::DieselFormatsLoadingParameters loadParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_LEGACY_CONSOLE);
  //loadParams.sourcePlatform = diesel::FileSourcePlatform::MICROSOFT_XBOX_360;

  //Reader xb360bdbr("E:\\torrented\\dieselgames\\payday-2-demo-xbox-360\\extract\\assets\\all.blb");
  //xb360bdbr.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(loadParams));
  //diesel::modern::BundleDatabase xb360bdb(xb360bdbr, loadParams);


  //diesel::modern::Bundle bundle("E:\\torrented\\payday-2-demo-xbox-360\\extract\\assets", "all", loadParams);


  //std::vector<diesel::modern::blobtypes::PackageBundle*> packages;

  //Reader testReader("E:\\torrented\\dieselgames\\payday-2-demo-xbox-360\\extract\\assets\\00d4033dd0221584_h.bundle");
  //testReader.SetSwapEndianness(true);
  ////diesel::modern::blobtypes::PackageBundle("E:\\torrented\\dieselgames\\payday-2-demo-xbox-360\\extract\\assets\\00d4033dd0221584_h.bundle", testReader, loadParams);
  //Reader test2;
  //testReader.ReadCompressedDataStore(test2);

  /*{
    Writer testWriter("./00d4033dd0221584_h.d.bundle");
    Reader testReader("X:\\SteamLibrary\\steamapps\\common\\RAID World War II - Modding\\assets\\bundle_db.blb");
    testWriter.SetSwapEndianness(false);
    testWriter.WriteReaderToCompressedDataStore(testReader);
    testWriter.Close();
  }

  {
    Reader testWriterReader("./00d4033dd0221584_h.d.bundle");
    Reader testWriterReader2;
    testWriterReader.ReadCompressedDataStore(testWriterReader2);

    Writer testWriterReaderWriter("./bundle_db_compresseddatastore_test.blb");
    testWriterReaderWriter.WriteReader(testWriterReader2);
    testWriterReaderWriter.Close();
    testWriterReader.Close();
  }*/

  //char* data = new char[test2.GetFileSize()];
  //test2.ReadBytesToBuffer(data, test2.GetFileSize());
  //
  //size_t compressedSize = compression::ZlibDecompression::GetRecommendedCompressionBufferSize(test2.GetFileSize());
  //
  //char* compressedData = new char[compressedSize];
  //compression::ZlibDecompression::CompressBuffer(data, test2.GetFileSize(), compressedData, compressedSize, ZLIB_COMPRESSION_LEVEL_DEFAULT_COMPRESSION);

  //return 0;

  //testWriter.WriteReader(test2);
  /*for (std::filesystem::recursive_directory_iterator i("E:\\torrented\\dieselgames\\payday-2-demo-xbox-360\\extract\\assets"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;
      if (i->path().string().find("stream") != std::string::npos)
        continue;
      if (i->path().string().find("_h") == std::string::npos)
        continue;

      Reader r(i->path());
      r.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(loadParams));
      packages.push_back(new diesel::modern::blobtypes::PackageBundle(i->path(), r, loadParams));
    }
  }*/


  /*std::filesystem::path outPath = "./pd2xb360/";

  std::map<unsigned int, std::string> propertymap;


  for (auto property : xb360bdb.GetProperties()) {
    std::string propstr;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(property.first, propstr);

    propertymap.insert({ property.second, propstr });
  }*/

  /*for (auto package : packages) {
    for (auto& resource : package->GetResources()) {
      if (resource.name == diesel::modern::Idstring("settings/render_templates"))
        __debugbreak();
    }
  }*/

  /*for (auto& file : xb360bdb.GetLookup()) {
    bool opened = false;
    Reader fileContents;

    //opened = bundle.open(fileContents, file.second);

    if (!opened) {
      for (auto package : packages) {
        opened = package->open(fileContents, file.second);
        if (opened)
          break;
      }
    }

    if (!opened)
      continue;

    std::string name;
    std::string type;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(file.first._name, name);
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(file.first._type, type);

    std::string fileName;

    fileName = name + ".";

    if (file.first._properties != 0) {
      for (auto& prop : propertymap) {
        if ((file.first._properties & prop.first) != 0) {
          fileName += prop.second + ".";
        }
      }
    }
    fileName += type;

    std::filesystem::path outFilePath = outPath / fileName;

    if (!std::filesystem::exists(outFilePath.parent_path()))
      std::filesystem::create_directories(outFilePath.parent_path());

    Writer outFileContents(outFilePath);
    if (file.first._type != diesel::modern::Idstring("lua")) {
      outFileContents.WriteReader(fileContents);
    }
    else {
      auto fs = fileContents.GetFileSize();
      char* buffer = new char[fs];
      fileContents.ReadBytesToBuffer(buffer, fs);


      for (int i = 0; i < fs; i++) {
        //int keyIndex = ((fs + i) * 7) % LuaDecryptionKeyLen;
        //buffer[i] ^= (char)(LuaDecryptionKey[keyIndex] * (fs - i));
        buffer[i] ^= LuaDecryptionKey[i % LuaDecryptionKeyLen];
      }

      outFileContents.WriteBytes(buffer, fs);

      delete[] buffer;

    }
    outFileContents.Close();
  }*/

  //return 0;

  //diesel::DieselFormatsLoadingParameters loadParams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::GRAW);

  //diesel::graw::DieselScript testdxe;
  //Reader testdxereader("E:\\Program Files (x86)\\Ubisoft\\Ghost Recon Advanced Warfighter\\Bundles\\patch\\data\\unit_ext\\factory.dxe");
  //testdxe.ReadCompiledDXE(testdxereader, loadParams);
  //Reader testdxereadersetup("E:\\Program Files (x86)\\Ubisoft\\Ghost Recon Advanced Warfighter\\Bundles\\quick\\data\\lib\\setups\\setup.dxe");
  //testdxe.ReadCompiledDXE(testdxereadersetup, loadParams);
  //Reader testdxereadersetup("E:\\Program Files (x86)\\Ubisoft\\Ghost Recon Advanced Warfighter\\Bundles\\quick\\data\\lib\\utils\\dev\\toolbox.dxe");
  //Reader testdxereadersetup("X:\\Projects\\DieselEngineExplorer\\test_files\\grawdxe\\test.dxe");
  //testdxe.ReadCompiledDXE(testdxereadersetup, loadParams);

  /*diesel::DieselFormatsLoadingParameters pdthps3loadparams = diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_THE_HEIST_V1);
  pdthps3loadparams.sourcePlatform = diesel::FileSourcePlatform::SONY_PLAYSTATION_3;

  Reader ps3bdbr("E:\\torrented\\dieselgames\\PAYDAY The Heist - NPEA00331\\NPEA00331\\USRDIR\\assets\\all.blb");
  ps3bdbr.SetSwapEndianness(true);
  diesel::modern::BundleDatabase ps3bdb(ps3bdbr, pdthps3loadparams);*/
  

  /*std::ofstream filelist("./pdthps3files.txt");
  for (auto& resource : ps3bdb.GetLookup()) {
    std::string name;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.first._name, name);

    //if (name.find("stone") == std::string::npos || name.find("cold") == std::string::npos)
    //  continue;

    std::string type;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.first._type, type);

    filelist << name << "." << type << "\n";

  }
  filelist.close();*/

  /*diesel::modern::Bundle bundle("E:\\torrented\\dieselgames\\PAYDAY The Heist - NPEA00331\\NPEA00331\\USRDIR\\assets", "all", pdthps3loadparams);


  std::vector<diesel::modern::blobtypes::PackageBundle*> packages;

  for (std::filesystem::recursive_directory_iterator i("E:\\torrented\\dieselgames\\PAYDAY The Heist - NPEA00331\\NPEA00331\\USRDIR\\assets"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("all") != std::string::npos)
        continue;
      if (i->path().string().find("stream") != std::string::npos)
        continue;

      Reader r(i->path());
      r.SetSwapEndianness(true);
      packages.push_back(new diesel::modern::blobtypes::PackageBundle(i->path(), r, pdthps3loadparams));
    }
  }


  std::filesystem::path outPath = "./pdthps3/";

  std::map<unsigned int, std::string> propertymap;


  for (auto property : ps3bdb.GetProperties()) {
    std::string propstr;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(property.first, propstr);

    propertymap.insert({ property.second, propstr });
  }

  for (auto package : packages) {
    for (auto& resource : package->GetResources()) {
      if (resource.name == diesel::modern::Idstring("settings/render_templates"))
        __debugbreak();
    }
  }

  for (auto& file : ps3bdb.GetLookup()) {
    bool opened = false;
    Reader fileContents;

    opened = bundle.open(fileContents, file.second);

    if (!opened) {
      for (auto package : packages) {
        opened = package->open(fileContents, file.second);
        if (opened)
          break;
      }
    }

    if (!opened)
      continue;

    std::string name;
    std::string type;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(file.first._name, name);
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(file.first._type, type);

    std::string fileName;

    fileName = name + ".";

    if (file.first._properties != 0) {
      for (auto& prop : propertymap) {
        if ((file.first._properties & prop.first) != 0) {
          fileName += prop.second + ".";
        }
      }
    }
    fileName += type;

    std::filesystem::path outFilePath = outPath / fileName;

    if (!std::filesystem::exists(outFilePath.parent_path()))
      std::filesystem::create_directories(outFilePath.parent_path());

    Writer outFileContents(outFilePath);
    if (file.first._type != diesel::modern::Idstring("lua")) {
      outFileContents.WriteReader(fileContents);
    }
    else {
      auto fs = fileContents.GetFileSize();
      char* buffer = new char[fs];
      fileContents.ReadBytesToBuffer(buffer, fs);


      for (int i = 0; i < fs; i++) {
        //int keyIndex = ((fs + i) * 7) % LuaDecryptionKeyLen;
        //buffer[i] ^= (char)(LuaDecryptionKey[keyIndex] * (fs - i));
        buffer[i] ^= LuaDecryptionKey[i % LuaDecryptionKeyLen];
      }

      outFileContents.WriteBytes(buffer, fs);

      delete[] buffer;

    }
    outFileContents.Close();
  }*/


  //return 0;
  /*std::vector<diesel::modern::blobtypes::PackageBundle*> packages;

  for (std::filesystem::recursive_directory_iterator i("X:\\SteamLibrary\\steamapps\\common\\PAYDAY 2\\assets"), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().extension() != ".bundle")
        continue;
      if (i->path().string().find("_h.bundle") == std::string::npos)
        continue;
      if (i->path().string().find("stream") != std::string::npos)
        continue;

      Reader r(i->path());
      packages.push_back(new diesel::modern::blobtypes::PackageBundle(i->path(), r, diesel::modern::ModernEngineVersion::PAYDAY_2_LATEST));
    }
  }

  for (auto& package : packages) {
    for (auto& resource : package->GetResources()) {
      if (resource.type == diesel::modern::Idstring("shaders")) {
        std::string name;
        diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.name, name);
        std::cout << name << std::endl;
      }
    }
  }*/

  /*Reader pd2linuxbdbr("X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\pd2linux\\pd2linuxassets\\assets\\bundle_db.blb");
  diesel::modern::BundleDatabase pd2linuxbdb(pd2linuxbdbr, diesel::modern::ModernEngineVersion::PAYDAY_2_LINUX_LATEST);
  pd2linuxbdbr.Close();


  Reader pd2linuxcoreenginer("X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\pd2linux\\pd2linuxassets\\assets\\3b6c2da5af6f2e42_h.bundle");
  diesel::modern::blobtypes::PackageBundle pd2linuxcoreengine("X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\pd2linux\\pd2linuxassets\\assets\\3b6c2da5af6f2e42_h.bundle", pd2linuxcoreenginer, diesel::modern::ModernEngineVersion::PAYDAY_2_LINUX_LATEST);
  pd2linuxcoreenginer.Close();

  std::map<unsigned int, std::string> properties;
  for (auto& property : pd2linuxbdb.GetProperties()) {
    std::string name;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(property.first, name);
    properties.insert({ property.second, name });
  }

  std::filesystem::path outPath = "X:\\Projects\\DieselEngineExplorer\\test_files\\shaders\\pd2linux";

  for (auto& resource : pd2linuxbdb.GetLookup()) {
    if (resource.first._type != diesel::modern::Idstring("shaders"))
      continue;

    std::string name;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(resource.first._name, name);

    auto fileOut = outPath / (std::filesystem::path(name).filename().string() + "." + properties[resource.first._properties] + ".shaders");

    Reader fileReader;
    if (pd2linuxcoreengine.open(fileReader, resource.second)) {
      Writer outFileWriter(fileOut);
      outFileWriter.WriteReader(fileReader);
      outFileWriter.Close();
    }

    std::cout << name << "(" << properties[resource.first._properties] << ")" << std::endl;
  }*/

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
  //return 0;
}