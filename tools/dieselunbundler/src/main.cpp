#include "fileio/reader.h"
#include "diesel/modern/modern_shared.h"
#include "diesel/modern/hashlist.h"
#include "diesel/modern/bundle.h"

#include <filesystem>
#include <map>
#include <unordered_map>
#include <string>
#include <iostream>

// For win32 file dialogs
#include <ShObjIdl.h>
#include <shtypes.h>
#include <wrl/client.h>
#pragma comment(lib, "ole32.lib")

using namespace diesel;

DieselFormatsLoadingParameters loadingParameters;
std::filesystem::path inputPath;
std::filesystem::path outputPath;



std::unordered_map<std::string, EngineVersion> validEngineVersions = {
  {"graw", EngineVersion::GRAW},
  {"graw2", EngineVersion::GRAW2},

  {"bioniccommando", EngineVersion::BIONIC_COMMANDO},
  {"bioniccommandorearmed", EngineVersion::BIONIC_COMMANDO_REARMED},
  {"bioniccommandorearmed1", EngineVersion::BIONIC_COMMANDO_REARMED},
  {"bioniccommandorearmed2", EngineVersion::BIONIC_COMMANDO_REARMED2},
  {"bc", EngineVersion::BIONIC_COMMANDO},
  {"bcr", EngineVersion::BIONIC_COMMANDO_REARMED},
  {"bcr1", EngineVersion::BIONIC_COMMANDO_REARMED},
  {"bcr2", EngineVersion::BIONIC_COMMANDO_REARMED2},

  {"wanted", EngineVersion::WANTED},
  {"terminator", EngineVersion::TERMINATOR_SALVATION},
  {"lag", EngineVersion::LEAD_AND_GOLD},
  {"pdthv1", EngineVersion::PAYDAY_THE_HEIST_V1},
  {"pdth", EngineVersion::PAYDAY_THE_HEIST_LATEST},
  {"pd2legacy", EngineVersion::PAYDAY_2_LEGACY},
  {"pd2", EngineVersion::PAYDAY_2_LATEST},
  {"pd2linux", EngineVersion::PAYDAY_2_LINUX_LATEST},

  {"pd2legacyconsole", EngineVersion::PAYDAY_2_LEGACY_CONSOLE},
  {"pd2modernconsole", EngineVersion::PAYDAY_2_MODERN_CONSOLE},
  {"raidww2", EngineVersion::RAID_WORLD_WAR_II_LATEST},
};

std::unordered_map<std::string, FileSourcePlatform> validSourcePlatforms = {
  {"win32", FileSourcePlatform::WINDOWS_32},
  {"win64", FileSourcePlatform::WINDOWS_64},
  {"linux64", FileSourcePlatform::LINUX_64},

  {"xbox360", FileSourcePlatform::MICROSOFT_XBOX_360},
  {"xb360", FileSourcePlatform::MICROSOFT_XBOX_360},
  {"xboxone", FileSourcePlatform::MICROSOFT_XBOX_ONE},
  {"xbone", FileSourcePlatform::MICROSOFT_XBOX_ONE},

  {"ps3", FileSourcePlatform::SONY_PLAYSTATION_3},
  {"playstation3", FileSourcePlatform::SONY_PLAYSTATION_3},
  {"ps4", FileSourcePlatform::SONY_PLAYSTATION_4},
  {"playstation4", FileSourcePlatform::SONY_PLAYSTATION_4},

  {"nintendoswitch", FileSourcePlatform::NINTENDO_SWITCH},
  {"ns", FileSourcePlatform::NINTENDO_SWITCH},
  {"switch", FileSourcePlatform::NINTENDO_SWITCH},
};

std::unordered_map<EngineVersion, std::string> readableEngineVersionNames = {

  {EngineVersion::GRAW, "Ghost Recon Advanced Warfighter"},
  {EngineVersion::GRAW2, "Ghost Recon Advanced Warfighter 2"},
  {EngineVersion::BIONIC_COMMANDO, "Bionic Commando"},
  {EngineVersion::BIONIC_COMMANDO_REARMED, "Bionic Command Rearmed"},
  {EngineVersion::BIONIC_COMMANDO_REARMED2, "Bionic Commando Rearmed 2"},
  {EngineVersion::WANTED, "Wanted: Weapons of Fate"},
  {EngineVersion::TERMINATOR_SALVATION, "Terminator Salvation"},
  {EngineVersion::LEAD_AND_GOLD, "Lead and Gold: Gangs of the Wild West"},
  {EngineVersion::PAYDAY_THE_HEIST_V1, "PAYDAY: The Heist V1"},
  {EngineVersion::PAYDAY_THE_HEIST_LATEST, "PAYDAY: The Heist"},
  {EngineVersion::PAYDAY_2_LEGACY, "PAYDAY 2 Legacy (Pre-Golden Grin)"},
  {EngineVersion::PAYDAY_2_LATEST, "PAYDAY 2"},
  {EngineVersion::PAYDAY_2_LINUX_LATEST, "PAYDAY 2 Linux"},
  {EngineVersion::PAYDAY_2_LEGACY_CONSOLE, "PAYDAY 2 Legacy Console (PS3/XBOX 360)"},
  {EngineVersion::PAYDAY_2_MODERN_CONSOLE, "PAYDAY 2 Modern Console (PS4/XBOX One/Nintendo Switch)"},
  {EngineVersion::RAID_WORLD_WAR_II_LATEST, "RAID: World War II"},
};

std::string trim_and_lower(std::string str)
{

  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c) {return !::isspace(c); }));
  str.erase(std::find_if(str.rbegin(), str.rend(), [](char c) {return !::isspace(c); }).base(), str.end());

  return str;
}

void get_game()
{
  printf("Please enter a valid game to extract for.\nValid options:\n");
  for (auto& game : validEngineVersions) {
    printf("\t\"%s\" - %s\n", game.first.c_str(), readableEngineVersionNames[game.second].c_str());
  }

  while (true) {
    printf("Game: ");
    std::string gamename;
    std::getline(std::cin, gamename);

    gamename = trim_and_lower(gamename);

    if (validEngineVersions.contains(gamename)) {
      loadingParameters.version = validEngineVersions[gamename];
      printf("Selected game: %s\n", readableEngineVersionNames[loadingParameters.version].c_str());
      break;
    }

    printf("Invalid game \"%s\"\n", gamename.c_str());
  }
}

void get_platform()
{
  printf("Please enter a valid platform to extract from. This should be the platform that the bundle files are from.\nValid options:\n");
  for (auto& platform : validSourcePlatforms) {
    printf("\t\"%s\"\n", platform.first.c_str());
  }

  while (true) {
    printf("Platform: ");
    std::string platform;
    std::getline(std::cin, platform);

    platform = trim_and_lower(platform);

    if (validSourcePlatforms.contains(platform)) {
      loadingParameters.sourcePlatform = validSourcePlatforms[platform];
      printf("Selected platform: %s\n", platform.c_str());
      break;
    }

    printf("Invalid platform \"%s\"\n", platform.c_str());
  }
}

std::filesystem::path win32filedialog()
{

  Microsoft::WRL::ComPtr<IFileDialog> fileDialog;
  //IFileDialog* fileDialog = NULL;

  HRESULT result = S_OK;
  std::wstring filePathStr = L"";

  if (!SUCCEEDED(CoInitialize(NULL)))
    return filePathStr;

  result = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog));
  if (!SUCCEEDED(result))
    return filePathStr;

  DWORD fileDialogFlags;
  result = fileDialog->GetOptions(&fileDialogFlags);
  if (!SUCCEEDED(result))
    return filePathStr;

  result = fileDialog->SetOptions(fileDialogFlags | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS);
  if (!SUCCEEDED(result))
    return filePathStr;

  result = fileDialog->Show(NULL);
  if (!SUCCEEDED(result))
    return filePathStr;

  Microsoft::WRL::ComPtr<IShellItem> resultObject;
  result = fileDialog->GetResult(&resultObject);
  if (!SUCCEEDED(result))
    return filePathStr;

  PWSTR filePath = NULL;
  result = resultObject->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
  if (!SUCCEEDED(result))
    return filePathStr;

  filePathStr = std::wstring(filePath);
  CoTaskMemFree(filePath);

  CoUninitialize();

  return filePathStr;
}

std::filesystem::path get_path(bool inputDir)
{
  printf("Please enter or drag and drop the %s path. Enter \"?\" if you wish to use a folder browser.\n", (inputDir ? "input" : "output"));

  while (true) {
    printf("Path: ");

    std::string path;
    std::getline(std::cin, path);

    if (path == "?") {
      std::filesystem::path selected = win32filedialog();
      if (selected != L"")
        return selected;
    } else if (std::filesystem::exists(path)) {
      return path;
    }
    else if (path != "") {
      printf("Path does not exist.\n");
    }
  }
}

void unpack_legacy();
void unpack_modern();

int main(int argc, char* argv[])
{
  setlocale(LC_ALL, ".utf-8");
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  std::locale::global(std::locale(".utf-8"));
  std::cin.imbue(std::locale());
  std::cout.imbue(std::locale());

  get_game();
  get_platform();
  inputPath = get_path(true);
  outputPath = get_path(false);

  printf("Starting extractor\n");
  printf("Chosen game: %s\n", readableEngineVersionNames[loadingParameters.version].c_str());
  wprintf(L"Input directory: %s\n", inputPath.wstring().c_str());
  wprintf(L"Output directory: %s\n", inputPath.wstring().c_str());

  if (loadingParameters.version > EngineVersion::MODERN_VERSION_START && (std::filesystem::exists("./hashlist") || std::filesystem::exists("./hashlist.txt"))) {
    printf("Loading hashlist from local folder\n");

    for (auto hashlistfile : { "./hashlist", "./hashlist.txt" }) {
      if (!std::filesystem::exists(hashlistfile))
        continue;
      Reader hashlistreader(hashlistfile);
      diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlistreader);
      hashlistreader.Close();
    }

    printf("Loaded hashlist\n");
  }

  if (loadingParameters.version > EngineVersion::MODERN_VERSION_START)
    unpack_modern();
  else
    unpack_legacy();

  return 0;
}

std::vector<std::filesystem::path> get_files_from_name(const std::wstring& contains, const std::vector<std::wstring> doesntContain)
{
  std::vector<std::filesystem::path> results;
  for (std::filesystem::recursive_directory_iterator i(inputPath), end; i != end; ++i) {
    if (!std::filesystem::is_directory(i->path())) {
      if (i->path().wstring().find(contains) == std::wstring::npos)
        continue;

      for (auto& doesnt : doesntContain) {
        if (i->path().wstring().find(doesnt) != std::wstring::npos)
          continue;
      }

      results.push_back(i->path());
    }
  }
  return results;
}

// quick.bundle, etc.
void unpack_legacy()
{

}

// modern streamed bundles + package bundles
void unpack_modern()
{
  std::vector<std::filesystem::path> packageHeaders;
  if (loadingParameters.version > EngineVersion::PAYDAY_THE_HEIST_V1) {
    packageHeaders = get_files_from_name(L"_h.bundle", {});
  }
  else {
    packageHeaders = get_files_from_name(L".bundle", {L"all", L"stream"});
  }

  std::filesystem::path bundleDatabasePath;

  if (std::filesystem::exists(inputPath / L"all.blb")) {
    bundleDatabasePath = inputPath / L"all.blb";
  }
  else if (std::filesystem::exists(inputPath / L"bundle_db.blb")) {
    bundleDatabasePath = inputPath / L"bundle_db.blb";
  }
  if (bundleDatabasePath == L"") {
    printf("ERROR: Couldn't find Bundle Database\n");
    return;
  }

  modern::BundleDatabase bdb;
  Reader bdbReader(bundleDatabasePath);
  bdb.Read(bdbReader, loadingParameters);
  bdbReader.Close();

  printf("Loading streamed bundles\n");
  modern::Bundle bundle(inputPath, "all", loadingParameters);
  printf("Loaded streamed bundles\n");

  std::vector<modern::PackageBundle*> packages;
  
  printf("Loading %zu packages\n", packageHeaders.size());
  for (auto& filePath : packageHeaders) {
    Reader packageReader(filePath);
    modern::PackageBundle* package = new modern::PackageBundle(filePath, packageReader, loadingParameters);
    packages.push_back(package);
  }
  printf("Loaded packages\n");


}
