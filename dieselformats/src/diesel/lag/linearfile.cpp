#include "diesel/lag/linearfile.h"

#include "fileio/zlibcompression.h"

#include <cassert>

const unsigned long long Diesel_LinearFile_BlockSize = 0x10000;

namespace diesel {
  namespace lag {
    LinearHeader::LinearHeader(Reader& reader) { // dsl::LinearFileManager::add_linear_header
      this->strings = StringTable(reader);

      auto num_entries = reader.ReadType<uint32_t>();

      assert(num_entries > 0);

      for (int i = 0; i < num_entries; i++) {
        auto directory = reader.ReadType<uint32_t>();
        auto basename = reader.ReadType<uint32_t>();
        auto size = reader.ReadType<uint32_t>();
        auto def_linear_file = reader.ReadType<uint32_t>();

        LinearFileEntry entry{};
        entry.directory = strings.GetString(directory);
        entry.basename = strings.GetString(basename);
        entry.size = size;
        entry.def_linear_file = def_linear_file;

        this->_entries.push_back(std::make_shared<LinearFileEntry>(entry));
      }

      auto num_linear_files = reader.ReadType<uint32_t>();

      for (int i = 0; i < num_linear_files; i++) {
        auto file_name = reader.ReadType<uint32_t>();
        auto num_files = reader.ReadType<uint32_t>();

        LinearFileHeader header{};

        header.index = i;
        header.file_name = strings.GetString(file_name);

        for (int j = 0; j < num_files; j++) {
          auto idx = reader.ReadType<uint32_t>();
          header.sorted_files.push_back(this->_entries[idx]);
        }
        for (int j = 0; j < num_files; j++) {
          auto off = reader.ReadType<uint32_t>();
          header.uncompressed_file_offset.push_back(off);
        }

        this->linear_file_headers.push_back(std::make_shared<LinearFileHeader>(header));
      }

      auto num_dirs = reader.ReadType<uint32_t>();
    }

    StringTable& LinearHeader::GetStringTable() {
      return this->strings;
    }

    const std::vector<LinearFileEntry>& LinearHeader::GetUniqueFiles() {
      //return this->uniqueFiles;
      return {};
    }

    std::vector<std::shared_ptr<LinearFileHeader>>& LinearHeader::GetLinearFileHeaders() {
      return this->linear_file_headers;
    }



    LinearFile::LinearFile(Reader& reader, std::shared_ptr<LinearFileHeader>& thisHeader) { // dsl::linearfile::LinearFile::LinearFile
      if (reader.GetFileSize() <= 0) {
        return;
      }

      this->header = thisHeader;

      auto segment_count = reader.ReadType<int32_t>();
      auto segment_size = reader.ReadType<int32_t>();

      for (int i = 0; i < segment_count; i++) {
        this->compressed_offsets.push_back(reader.ReadType<int32_t>());
        this->compressed_sizes.push_back(reader.ReadType<int32_t>());
      }
    }

    unsigned long long LinearFile::GetSortedFilesIndexOfFile(const std::string& directory, const std::string& basename) {
      
      for(int i = 0; i < this->header->sorted_files.size(); i++) {
        auto sortedFile = this->header->sorted_files[i];
        if (sortedFile->directory == directory && sortedFile->basename == basename)
        {
          return i;
        }
      }

      return -1;
    }
    std::size_t LinearFile::GetSizeOfEntry(const std::string& directory, const std::string& basename) {
      auto index = this->GetSortedFilesIndexOfFile(directory, basename);
      if(index != -1)
        return this->header->sorted_files[index]->size;
      return -1;
    }
    void LinearFile::GetFlatListOfEntries(std::vector<LinearFileDirectoryBasenamePair>& out) {
      if (!this->header)
        return;

      for (const auto& entry : this->header->sorted_files) {
        if (entry->IsDirectory())
          continue;
        out.push_back({entry->directory, entry->basename});
      }
    }

    bool LinearFile::ReadEntryToReader(Reader& sourceLinearFileReader, Reader& outFileContentsReader, const std::string& directory, const std::string& basename) { // dsl::linearfile::LinearFile::open_immediate
      auto sortedIndex = GetSortedFilesIndexOfFile(directory, basename);

      if (sortedIndex == -1)
        return false;

      auto& fileEntry = this->header->sorted_files[sortedIndex];

      auto uncompressed_file_offset = this->header->uncompressed_file_offset[sortedIndex];

      auto start_offset = (uint16_t)uncompressed_file_offset;
      auto end_offset = (uint16_t)(LOWORD(fileEntry->size) + uncompressed_file_offset);

      auto start_segment = uncompressed_file_offset >> 16;
      auto end_segment = (fileEntry->size + uncompressed_file_offset) >> 16;

      auto start_block_offset = this->compressed_offsets[start_segment];

      sourceLinearFileReader.SetPosition(start_block_offset);

      assert(start_segment >= 0 && end_segment >= 0 && start_segment < this->compressed_offsets.size() && end_segment < this->compressed_offsets.size());

      char* segment_mem = new char[Diesel_LinearFile_BlockSize];
      char* uncompressed_block = new char[Diesel_LinearFile_BlockSize];

      char* file_contents = new char[fileEntry->size];
      std::size_t file_contents_position = 0;

      int current_segment = start_segment;
      while(current_segment <= end_segment) {
        sourceLinearFileReader.ReadBytesToBuffer(segment_mem, this->compressed_sizes[current_segment]);

        auto end_pos = end_offset;

        auto v582 = (current_segment != start_segment) ? 0 : uncompressed_file_offset;

        if (current_segment != end_segment)
          end_pos = Diesel_LinearFile_BlockSize;
        auto v58 = end_pos - v582;

        if (this->compressed_sizes[current_segment] == Diesel_LinearFile_BlockSize) {
          memcpy(&file_contents[file_contents_position], segment_mem, v58);
        }
        else {
          compression::ZlibDecompression::DecompressBuffer(segment_mem, *(int*)&segment_mem[this->compressed_sizes[current_segment] - 4], uncompressed_block, this->compressed_sizes[current_segment]);
          memcpy(&file_contents[file_contents_position], &uncompressed_block[v582], v58);
        }

        file_contents_position += v58;
        current_segment++;
      }

      delete[] segment_mem;
      delete[] uncompressed_block;

      outFileContentsReader = Reader(file_contents, fileEntry->size);
    }

}
}
