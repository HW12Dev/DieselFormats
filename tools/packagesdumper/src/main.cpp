#include "diesel/modern/bundle.h"

#include "diesel/modern/hashlist.h"

#include <filesystem>
#include <fstream>

int main(int argc, char* argv[]) {
  diesel::EngineVersion engineVersion = diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST;
  std::filesystem::path dir = "";

  if (argc < 2) {
    printf("Usage: DieselPackagesDumper [game directory]\n");
    return 1;
  }
  dir = argv[1];

  if (!std::filesystem::is_directory(dir)) {
    printf("Error: %s is not a directory!\n", dir.string().c_str());
  }

  printf("Loading hashlist\n");
  if (std::filesystem::exists("./hashlist.txt")) {
    Reader hashlistReader("./hashlist.txt");
    diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlistReader);
    hashlistReader.Close();
  }
  if (std::filesystem::exists("./hashlist")) {
    Reader hashlistReader("./hashlist");
    diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlistReader);
    hashlistReader.Close();
  }
  printf("Loaded hashlist\n");


  std::ofstream packages("packages.txt");
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

      auto name = i->path().filename();
      name.replace_extension("");
      auto nameStr = name.string().substr(0, 16);

      auto idstringName = diesel::modern::Idstring(_byteswap_uint64(std::stoull(nameStr, nullptr, 16)));
      packages << "@" << diesel::modern::GetGlobalHashlist()->GetIdstringSource(idstringName) << "\n";

      Reader reader1(i->path());
      diesel::modern::blobtypes::PackageBundle package(i->path(), reader1, engineVersion);
      reader1.Close();

      for (auto& resource : package.GetResources()) {
        packages << diesel::modern::GetGlobalHashlist()->GetIdstringSource(resource.name) << "." << diesel::modern::GetGlobalHashlist()->GetIdstringSource(resource.type) << "\n";
      }
      packages.flush();
    }
  }
  packages.close();

  printf("Successfully wrote package.txt\n");

  return 0;
}