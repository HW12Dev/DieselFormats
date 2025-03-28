#pragma once

#include "fileio/reader.h"
#include "diesel/lag/stringtable.h"

namespace diesel {
  namespace lag {
    typedef std::pair<std::filesystem::path, std::filesystem::path> LinearFileDirectoryBasenamePair;

    class LinearFileEntry {
    public:
      bool IsDirectory() const;
    public:
      std::string directory;
      std::string basename;
      int size;
      int def_linear_file;
    };

    class LinearFileHeader {
    public:
      unsigned int index;
      std::string file_name;
      std::vector<LinearFileEntry*> sorted_files;
      std::vector<unsigned int> uncompressed_file_offset;
    };

    class LinearHeader {
    public:
      LinearHeader(Reader& reader);

    public:
      StringTable& GetStringTable();
      
      const std::vector<LinearFileEntry>& GetUniqueFiles();
      std::vector<LinearFileHeader>& GetLinearFileHeaders();

      

    private:
      StringTable strings;
      std::vector<LinearFileEntry> _entries;
      std::vector<LinearFileHeader> linear_file_headers;
    };

    class LinearFileOptions {
      int _read_block_count;
      int _read_block_size;
      int _num_decompress_blocks;
    };
    class LinearFile {
    public:
      LinearFile(Reader& reader, LinearFileHeader& thisHeader);

    private:
      unsigned long long GetSortedFilesIndexOfFile(const std::string& directory, const std::string& basename);
    public:
      std::size_t GetSizeOfEntry(const std::string& directory, const std::string& basename);

      void ReadEntryToBuffer(Reader& reader, const std::string& directory, const std::string& basename, char* outBuffer, std::size_t outBufferSize);

    public:
      void GetFlatListOfEntries(std::vector<LinearFileDirectoryBasenamePair>& out);
    private:
      LinearFileHeader* header;

      std::vector<int> compressed_offsets;
      std::vector<int> compressed_sizes;
    };
  }
}