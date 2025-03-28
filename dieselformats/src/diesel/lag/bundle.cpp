#include "diesel/lag/bundle.h"
#include "bundle.h"
#include "bundle.h"
#include "bundle.h"
#include "bundle.h"
#include "bundle.h"
#include "bundle.h"

namespace diesel {
  namespace lag {
    FileBundle::FileBundle(Reader& reader) {
      reader.ReadBytesToBuffer((char*)&this->header, sizeof(this->header));

      this->files_populated = false;

      this->LoadDirRecursive(reader, &this->_root_dir);
    }

    void FileBundle::LoadDirRecursive(Reader& reader, DirectoryType* dir) {
      Entry entry;
      char type = 0;
      if (this->header.version == 2) {
        type = reader.ReadType<char>();
        for (auto i = type; type != 3; i = type) {

          if (type == 1) { // entry is a directory
            entry.exists = reader.ReadType<char>();
            entry.str = reader.ReadString();

            //bool exists = false;
            entry.file_start = 0;
            entry.file_size = 0;


            if (dir) {
              auto pair = dir->emplace(entry.str, entry);
              this->LoadDirRecursive(reader, &pair.first->second.directory);
            }
          }
          else if (type == 2) { // entry is a file in a directory

            entry.file_start = reader.ReadType<uint64_t>();
            entry.file_size = reader.ReadType<int>();
            entry.exists = reader.ReadType<char>();

            entry.str = reader.ReadString();

            if (dir)
              dir->insert({ entry.str, entry });

          }
          type = reader.ReadType<char>();
        }
      }
      else if (this->header.version == 1) {

      }
    }

    FileBundle::InternalFileListType& FileBundle::GetFileList() {
      this->PopulateInternalFileList();

      return this->files;
    }

    std::size_t FileBundle::GetEntrySize(const std::filesystem::path& path) {
      auto find = this->files.find(path);

      if (find != this->files.end()) {
        return find->second->file_size;
      }

      return -1;
    }

    void FileBundle::ReadEntryToBuffer(Reader& reader, const std::filesystem::path& path, char* outBuffer, std::size_t outBufferSize) {
      auto find = this->files.find(path);

      if (find == this->files.end())
        return;

      reader.SetPosition(find->second->file_start);

      reader.ReadBytesToBuffer(outBuffer, outBufferSize);
    }

    void FileBundle::PopulateInternalFileList() {
      if (!this->files_populated)
        this->RecursivelyAddFullFilePathsAndEntriesToMap(this->_root_dir, this->files, "");
      this->files_populated = true;
    }


    void FileBundle::RecursivelyAddFullFilePathsAndEntriesToMap(DirectoryType& directory, InternalFileListType& out, std::string currentPath) {
      for(auto& entry : directory) {
        if (entry.second.file_start != 0) { // file
          auto addEntry = currentPath + "/" + entry.first;
          if (addEntry.starts_with("/")) {
            addEntry = addEntry.substr(1, addEntry.size() - 1); // remove leading slash ("/") from string
          }
          out.insert({ std::filesystem::path(addEntry), &entry.second });
        }
        else {
          std::string currentPath2 = currentPath + "/" + entry.first;
          RecursivelyAddFullFilePathsAndEntriesToMap(entry.second.directory, out, currentPath2);
        }
      }
    }

  }
}
