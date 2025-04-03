#include "fileio/reader.h"

#include <filesystem>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cout << "Usage: BANDITSDecryptor input_file.xml.enc [optional_out.xml]\nOr: Drag and drop an encrypted BANDITS file onto this tool and the output will be saved to the same folder as the input.\nProviding an unencrypted file as the input for this program will save the encrypted version to the output.\n";
    return 0;
  }

  std::filesystem::path inputEnc = std::filesystem::path(argv[1]);

  std::wstring autoOutputFile = inputEnc.wstring();
  {
    auto encFind = autoOutputFile.find(L".enc");
    if(encFind != std::wstring::npos)
      autoOutputFile = autoOutputFile.replace(encFind, encFind + sizeof(L".enc"), L"");
  }

  std::filesystem::path outputFile = std::filesystem::path(argc > 2 ? argv[2] : std::filesystem::path(autoOutputFile));

  if (!std::filesystem::exists(inputEnc)) {
    std::cout << "Input file: " << std::filesystem::absolute(inputEnc) << " does not exist.\n";
    return 1;
  }

  if (std::filesystem::absolute(inputEnc) == std::filesystem::absolute(outputFile)) {
    std::filesystem::path file = outputFile;
    std::wstring extension = file.extension();
    file = file.replace_extension("");
    std::wstring outFileName = file.filename().wstring();
    outFileName += L" new";
    
    outputFile = outputFile.parent_path() / (outFileName + extension);
  }

  Reader inputReader(inputEnc);
  Reader decrypted;
  inputReader.ReadBANDITSEncryptedFile(decrypted);
  inputReader.Close();

  char* data = new char[decrypted.GetFileSize()] {};
  decrypted.ReadBytesToBuffer(data, decrypted.GetFileSize());

  std::ofstream outStream(outputFile, std::ios::binary);
  outStream.write(data, decrypted.GetFileSize());
  delete[] data;

  return 0;
}