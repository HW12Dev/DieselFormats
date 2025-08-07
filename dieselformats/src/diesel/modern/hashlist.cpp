#include "diesel/modern/hashlist.h"

namespace diesel {
  namespace modern {
    Hashlist GlobalHashlist = Hashlist();
    Hashlist* GetGlobalHashlist() {
      return &GlobalHashlist;
    }

    bool Hashlist::FindSourceForIdstring(const Idstring& id, std::string& out) {
      auto find = this->hashes.find(id);

      if (find != this->hashes.end()) {
        out.assign(find->second);
        return true;
      }
      out.assign(id.hex());
      return false;
    }

    std::string Hashlist::GetIdstringSource(const Idstring& id, bool* success)
    {
      std::string str;
      bool success_ = FindSourceForIdstring(id, str);
      if (success)
        *success = success_;
      return str;
    }

    void Hashlist::AddSourceToHashlist(const std::string& str) {
      this->AddSourceToHashlist(str.c_str());
    }
    void Hashlist::AddSourceToHashlist(const char* str) {
      this->hashes.insert({ Idstring(str), std::string(str) });
    }
    void Hashlist::ReadFileToHashlist(Reader& reader) {
      auto hashlistRawFileSize = reader.GetFileSize();
      char* hashlistBuf = new char[hashlistRawFileSize + 1];
      hashlistBuf[hashlistRawFileSize] = '\x00';
      reader.ReadBytesToBuffer(hashlistBuf, hashlistRawFileSize);
      Reader hashlistReader(hashlistBuf, hashlistRawFileSize+1);

      while (!hashlistReader.AtEndOfBuffer()) {
        std::string processed_str;

        char c = hashlistReader.ReadType<char>();
        while (c != '\n' && c != '\r' && c != '\x00') {
          processed_str += c;
          c = hashlistReader.ReadType<char>();
        }
        if (processed_str == "")
          continue;


        this->AddSourceToHashlist(processed_str);
        processed_str = "";
      }
    }
    void Hashlist::DumpHashlistToFile(Writer& writer) {
      for (auto& hash : this->hashes) {
        std::string toWrite = hash.second + "\n";
        writer.WriteBytes((char*)toWrite.c_str(), toWrite.size());;
      }
    }
  }
}
