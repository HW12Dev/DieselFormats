#pragma once

#include "fileio/reader.h"

#include <map>

namespace diesel {
  namespace lag {
    class FileBundle {
    public:
      struct Header {
        char tag[4];
        int version;
        int size;
        int reserved;
      };

      class Entry;
      typedef std::map<std::string, Entry> DirectoryType;
      class Entry {
      public:
        std::map<std::string, Entry> directory;

        long long file_start;
        int file_size;
        bool exists;
        std::string str;
      };

    public:
      FileBundle(Reader& reader);

    public:
      typedef std::map<std::filesystem::path, Entry*> InternalFileListType;

      InternalFileListType& GetFileList();

      std::size_t GetEntrySize(const std::filesystem::path& path);
      void ReadEntryToBuffer(Reader& reader, const std::filesystem::path& path, char* outBuffer, std::size_t outBufferSize);
    private:
      void PopulateInternalFileList();

      void RecursivelyAddFullFilePathsAndEntriesToMap(DirectoryType& directory, InternalFileListType& out, std::string currentPath);

      bool files_populated;
      InternalFileListType files;

    private:
      void LoadDirRecursive(Reader& reader, DirectoryType* dir);
    private:
      DirectoryType _root_dir;
      Header header;
    };
  }
}