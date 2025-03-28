#include "diesel/lag/linearfile.h"

#include "fileio/zlibcompression.h"

#include <cassert>

const unsigned long long BlockSize = 0x10000;

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

        this->_entries.push_back(entry);
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
          header.sorted_files.push_back(&this->_entries.data()[idx]);
        }
        for (int j = 0; j < num_files; j++) {
          auto off = reader.ReadType<uint32_t>();
          header.uncompressed_file_offset.push_back(off);
        }

        this->linear_file_headers.push_back(header);
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

    std::vector<LinearFileHeader>& LinearHeader::GetLinearFileHeaders() {
      return this->linear_file_headers;
    }



    LinearFile::LinearFile(Reader& reader, LinearFileHeader& thisHeader) { // dsl::linearfile::LinearFile::LinearFile
      if (reader.GetFileSize() <= 0) {
        return;
      }

      this->header = &thisHeader;

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

    void LinearFile::ReadEntryToBuffer(Reader& reader, const std::string& directory, const std::string& basename, char* outBuffer, std::size_t outBufferSize) { // dsl::linearfile::LinearFile::open_immediate
      throw std::runtime_error("This function does not run because it is terribly broken.");

      auto sortedIndex = this->GetSortedFilesIndexOfFile(directory, basename);

      if (sortedIndex == -1)
        return;

      auto fileEntry = this->header->sorted_files[sortedIndex];

      
      int start_block_index = HIWORD(this->header->uncompressed_file_offset[sortedIndex]);
      int end_block_index = HIWORD(fileEntry->size + this->header->uncompressed_file_offset[sortedIndex]);

      int uncompressed_data_write_offset = 0;
      int uncompressed_file_size = fileEntry->size;

      char* uncompressed_buffer = new char[uncompressed_file_size];

      reader.SetPosition(this->compressed_offsets[start_block_index]);

      char* current_read_block_storage = new char[BlockSize];
      char* temp_decompressed = new char[BlockSize];
      for (int current_block_index = start_block_index; current_block_index < end_block_index; current_block_index++) {
        auto size_of_current_block = this->compressed_sizes[current_block_index];

        reader.ReadBytesToBuffer(current_read_block_storage, size_of_current_block);

        auto data_start = current_block_index != start_block_index ? 0 : LOWORD(this->header->uncompressed_file_offset[sortedIndex]);

        auto decompressed_size = *(int*)&current_read_block_storage[size_of_current_block - 4];

        if (size_of_current_block == BlockSize) {
          // copy buffer directly
          memcpy(&uncompressed_buffer[uncompressed_data_write_offset], &current_read_block_storage[data_start], size_of_current_block);

          uncompressed_data_write_offset += size_of_current_block;
        }
        else {
          compression::ZlibDecompression::DecompressBuffer(current_read_block_storage, size_of_current_block, temp_decompressed, decompressed_size);

          memcpy(&uncompressed_buffer[uncompressed_data_write_offset], &temp_decompressed[data_start], decompressed_size);

          uncompressed_data_write_offset += decompressed_size;
        }
      }

      //int decompress_offset = 0;
      //auto uncompressed_file_offset_of_sorted_index = this->header->uncompressed_file_offset[sortedIndex];
      //
      //auto uncompressed_file_offset = HIWORD(uncompressed_file_offset_of_sorted_index);
      //int start_offset = uncompressed_file_offset_of_sorted_index;
      //int end_offset = fileEntry->size + uncompressed_file_offset_of_sorted_index;
      //
      //int start_segment = HIWORD(uncompressed_file_offset_of_sorted_index);
      //int end_segment = (fileEntry->size + uncompressed_file_offset_of_sorted_index) >> 16;
      //
      //char* segment_mem = new char[BlockSize];
      //char* uncompressed_block = new char[BlockSize];
      //
      //auto uncompressedaBufferSize = fileEntry->size;
      //if (!uncompressedaBufferSize)
      //  uncompressedaBufferSize = 1;
      //char* uncompresseda = new char[uncompressedaBufferSize];
      //
      //auto start = this->compressed_offsets[uncompressed_file_offset];
      //
      //reader.SetPosition(start);
      //
      //auto v26 = (unsigned __int16)uncompressed_file_offset_of_sorted_index;
      //
      //int v44 = uncompressed_file_offset;
      /*for (int v44 = uncompressed_file_offset; v44 <= end_segment; v44++) {
        reader.ReadBytesToBuffer(segment_mem, this->compressed_offsets[v44]);

        auto end_pos = end_offset;
        auto start = v44 != uncompressed_file_offset ? 0 : v26;

        if (v44 != end_segment)
          end_pos = BlockSize;

        auto v58 = end_pos - start;

        if (this->compressed_sizes[v44] == BlockSize) {
          memcpy(&uncompresseda[decompress_offset], &segment_mem[start], v58);
        }
        else {
          auto decompresed_size = *(int*)&segment_mem[this->compressed_sizes[v44] - 4];

          compression::ZlibDecompressionContext decompress = compression::ZlibDecompressionContext();

          decompress.DecompressBuffer(segment_mem, this->compressed_sizes[v44], uncompressed_block, decompresed_size);

          memcpy(&uncompresseda[decompress_offset], &uncompressed_block[start], v58);
        }
        decompress_offset += v58;
        v26 = start_offset;
        uncompressed_file_offset = start_segment;
      }*/

      //if (segment_mem)
      //  delete[] segment_mem;
      //if (uncompressed_block)
      //  delete[] uncompressed_block;
      //
      //if (uncompresseda) {
      //  memcpy(outBuffer, uncompresseda, outBufferSize);
      //  delete[] uncompresseda;
      //}
    }

    bool LinearFileEntry::IsDirectory() const {
      return this->def_linear_file == -1;
    }
}
}
