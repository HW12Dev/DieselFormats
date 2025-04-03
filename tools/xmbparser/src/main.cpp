#include "diesel/lag/xml.h"

#include <filesystem>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cout << "Usage: DieselXMBParser file.xmb [optional_out.xml]\nOr: Drag and drop a Diesel XMB file onto this tool and the output will be saved to the same folder as the input.\n";
    return 0;
  }

  std::filesystem::path inputXmb = std::filesystem::path(argv[1]);
  std::filesystem::path outputXml = std::filesystem::path(argc > 2 ? argv[2] : std::filesystem::path(inputXmb).replace_extension(".xml"));

  if (!std::filesystem::exists(inputXmb)) {
    std::cout << "Input file: " << std::filesystem::absolute(inputXmb) << " does not exist.\n";
    return 1;
  }

  Reader xmbReader(inputXmb);
  diesel::lag::XMLDocument xmb;
  if (xmb.ReadFromBinary(xmbReader, diesel::EngineVersion::LEAD_AND_GOLD) == false) {
    xmbReader.Close();
    std::cout << "Input file: " << std::filesystem::absolute(inputXmb) << " is not a valid Diesel XMB file!\n";
    return 2;
  }
  xmbReader.Close();

  std::ofstream outStream(outputXml);
  outStream << xmb.DumpRootToString();
  outStream.close();

  std::cout << "Successfully wrote XMB: " << inputXmb << " to XML: " << outputXml << "\n";
  return 0;
}