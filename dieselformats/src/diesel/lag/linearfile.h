#pragma once

#include "fileio/reader.h"
#include "diesel/lag/stringtable.h"

#include <memory>

namespace diesel {
  namespace lag {
    typedef std::pair<std::filesystem::path, std::filesystem::path> LinearFileDirectoryBasenamePair;

    class LinearFileEntry {
    public:
      bool IsDirectory() const { return this->def_linear_file == -1; }
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
      std::vector<std::shared_ptr<LinearFileEntry>> sorted_files;
      std::vector<unsigned int> uncompressed_file_offset;
    };

    class LinearHeader {
    public:
      LinearHeader(Reader& reader);

    public:
      StringTable& GetStringTable();
      
      const std::vector<LinearFileEntry>& GetUniqueFiles();
      std::vector<std::shared_ptr<LinearFileHeader>>& GetLinearFileHeaders();

      

    private:
      StringTable strings;
      std::vector<std::shared_ptr<LinearFileEntry>> _entries;
      std::vector<std::shared_ptr<LinearFileHeader>> linear_file_headers;
    };

    class LinearFileOptions {
      int _read_block_count;
      int _read_block_size;
      int _num_decompress_blocks;
    };
    class LinearFile {
    public:
      LinearFile(Reader& reader, std::shared_ptr<LinearFileHeader>& thisHeader);

    private:
      unsigned long long GetSortedFilesIndexOfFile(const std::string& directory, const std::string& basename);
    public:
      std::size_t GetSizeOfEntry(const std::string& directory, const std::string& basename);

      bool ReadEntryToReader(Reader& sourceLinearFileReader, Reader& outFileContentsReader, const std::string& directory, const std::string& basename);

    public:
      void GetFlatListOfEntries(std::vector<LinearFileDirectoryBasenamePair>& out);
    private:
      std::shared_ptr<LinearFileHeader> header;

      std::vector<int> compressed_offsets;
      std::vector<int> compressed_sizes;
    };
  }
}