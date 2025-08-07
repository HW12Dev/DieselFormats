#pragma once

#include "diesel/modern/hash.h"
#include "fileio/reader.h"

#include <map>

namespace diesel {
  namespace modern {
    class Hashlist;
    Hashlist* GetGlobalHashlist();

    class Hashlist {
    public:
      Hashlist() { this->AddSourceToHashlist(""); }

    public:
      /// <summary>
      /// Attempts to find a source for an idstring, if the idstring is not present in the hashlist, this function returns false and places a hexidecimal version into "out".
      /// If successful this function returns true. And copies the found source string into "out".
      /// </summary>
      /// <param name="id"></param>
      /// <param name="out"></param>
      /// <returns></returns>
      bool FindSourceForIdstring(const Idstring& id, std::string& out);

      std::string GetIdstringSource(const Idstring& id, bool* success = nullptr);

      void AddSourceToHashlist(const std::string& str);
      void AddSourceToHashlist(const char* str);

      void ReadFileToHashlist(Reader& reader);
      void DumpHashlistToFile(Writer& writer);

    private:
      std::map<Idstring, std::string> hashes;
    };
  }
}