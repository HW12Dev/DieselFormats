#include "diesel/modern/bundle.h"
#include "fileio/zlibcompression.h"

#include <cassert>

// REMOVE
#include <fstream>

namespace diesel {
  namespace modern {

    bool diesel::modern::ResourceID::operator<(const diesel::modern::ResourceID& b) const {
      if (this->type != b.type) {
        if (this->type > b.type)
          return false;
        
        if (this->type >= b.type)
          return this->type < b.type;

        return true;
      }

      if (this->name > b.name)
        return false;
      if (this->name < b.name)
        return true;

      return this->name < b.name;
    }

    bool diesel::modern::DBExtKey::operator<(const DBExtKey& b) {

      if (this->_type <= b._type) {
        if (this->_type < b._type) return true;

        auto v7 = b._name;
        auto v8 = this->_name;
        auto v9 = v7;
        auto v10 = v8;

        if (v8 <= v7
          && (v8 < v7 || v10 < v9 || v8 <= v7 && this->_properties < b._properties)) return 1;
      }
      return false;
    }


#pragma region MultiFileTransport

    MultiFileTransport::MultiFileTransport(const std::filesystem::path& basePath, const DieselFormatsLoadingParameters& version) : Transport(version) {
      this->basePath = basePath;
    }

    bool MultiFileTransport::open(Reader& outReader, unsigned int dbKey) {
      auto file = basePath / (std::string("all") + std::to_string(dbKey % 512)) / std::to_string(dbKey);

      if (std::filesystem::exists(file)) {
        outReader = Reader(file);
        return true;
      }


      return false;
    }

#pragma endregion

#pragma region PackageBundle

    const std::vector<diesel::modern::ResourceID>& PackageBundle::GetResources() { return this->resources; }
    PackageBundle::PackageBundle(const std::filesystem::path& source, Reader& inReader, const DieselFormatsLoadingParameters& version) : Transport(version) { // merge of PackageBundle::PackageBundle and PackageBundle::resources
      this->sourceFile = source;

      Reader reader;
      if (version.version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST || version.version == diesel::EngineVersion::PAYDAY_2_LEGACY_CONSOLE || version.version == diesel::EngineVersion::BIONIC_COMMANDO_REARMED2 || version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
        inReader.ReadCompressedDataStore(reader);
      }
      else {
        reader = inReader;
      }

      reader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));

      if (reader.GetFileSize() == 0)
        return;

      auto start = reader.GetPosition();
      auto size = reader.ReadType<uint32_t>();

      if(version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE)
        reader.AddPosition(4);

      diesel::modern::Vector<diesel::modern::Pair<unsigned int, unsigned int>> _header(reader, version);

      if (_header._data != 0) {
        if (version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
          _header._data += 4;
        }

        reader.SetPosition(start + 4 + _header._data); // start + size field size + _header._data

        for (int i = 0; i < _header._size; i++) {
          auto db_key = reader.ReadType<uint32_t>();
          auto unk1 = reader.ReadType<uint32_t>();

          this->header.push_back(std::make_pair(db_key, unk1));
        }
      }
      else {
        reader.SetPosition(start + size + 4);
      }

      assert(reader.ReadType<uint32_t>() == TypeId_BundleHeader); // header typeid (TypeId_BundleHeader)

      start = reader.GetPosition();

      size = reader.ReadType<uint32_t>();

      if (version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE)
        reader.AddPosition(4);

      diesel::modern::Vector<ResourceID> _resources(reader, version);

      if (_resources._data != 0) {
        if (version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
          _resources._data += 4;
        }
        reader.SetPosition(start + 4 + _resources._data); // start + size field size + _resources._data


        for (int i = 0; i < _resources._size; i++) {
          diesel::modern::ResourceID resource{};
          resource.type = reader.ReadType<uint64_t>();
          resource.name = reader.ReadType<uint64_t>();

          this->resources.push_back(resource);
        }
      }
      else {
        reader.SetPosition(start + size + 4);
      }

      assert(reader.ReadType<uint32_t>() == TypeId_PackageBundle); // resources typeid (TypeId_PackageBundle)

      if (reader.AtEndOfBuffer())
        return;

      size_t streamTypesSize = (version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) ? reader.ReadType<uint64_t>() : reader.ReadType<uint32_t>();

      for (int i = 0; i < streamTypesSize; i++) {
        this->stream_types.insert(reader.ReadType<uint64_t>());
      }
    }
    bool PackageBundle::Write(Writer& writer, const DieselFormatsLoadingParameters& version) {
      writer.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));

      auto headerSizePosition = writer.GetPosition();
      writer.WriteType<uint32_t>(0);

      // _header
      uint64_t headerDataPosition = 0;
      diesel::modern::Vector<diesel::modern::Pair<unsigned int, unsigned int>>::Write(writer, version, this->header.size(), this->header.size(), headerDataPosition);

      auto headerData = writer.GetPosition();

      for (int i = 0; i < this->header.size(); i++) {
        writer.WriteType<uint32_t>(this->header[i].first);
        writer.WriteType<uint32_t>(this->header[i].second);
      }

      writer.WriteType<uint32_t>(diesel::modern::TypeId_BundleHeader);

      auto headerSize = writer.GetPosition();

      auto resourcesSizePosition = writer.GetPosition();
      writer.WriteType<uint32_t>(0);

      uint64_t resourcesDataPosition = 0;
      diesel::modern::Vector<ResourceID>::Write(writer, version, this->resources.size(), this->resources.size(), resourcesDataPosition);

      auto resourcesData = writer.GetPosition();

      for (int i = 0; i < this->resources.size(); i++) {
        writer.WriteType<uint64_t>(this->resources[i].type);
        writer.WriteType<uint64_t>(this->resources[i].name);
      }

      writer.WriteType<uint32_t>(diesel::modern::TypeId_PackageBundle);

      auto resourcesSize = writer.GetPosition();

      writer.WriteType<uint32_t>((uint32_t)this->stream_types.size());

      for (auto& streamed : this->stream_types) {
        writer.WriteType<uint64_t>(streamed);
      }

      headerSize -= sizeof(uint32_t); // size header
      headerData -= sizeof(uint32_t); // size header
      resourcesSize = resourcesSize - headerSize - sizeof(uint32_t) - sizeof(uint32_t); // size of previous chunk - size header of previous chunk - size header of current chunk
      resourcesData = resourcesData - headerSize - sizeof(uint32_t) - sizeof(uint32_t); // size of previous chunk - size header of previous chunk - size header of current chunk


      if (this->resources.size() == 0)
        resourcesData = 0;
      if (this->header.size() == 0)
        headerData = 0;

      writer.SetPosition(headerSizePosition);
      if (diesel::AreLoadParameters32Bit(version))
        writer.WriteType<uint32_t>(headerSize);
      else
        writer.WriteType<uint64_t>(headerSize);

      writer.SetPosition(resourcesSizePosition);
      if (diesel::AreLoadParameters32Bit(version))
        writer.WriteType<uint32_t>(resourcesSize);
      else
        writer.WriteType<uint64_t>(resourcesSize);

      writer.SetPosition(headerDataPosition);
      if (diesel::AreLoadParameters32Bit(version))
        writer.WriteType<uint32_t>(headerData);
      else
        writer.WriteType<uint64_t>(headerData);

      writer.SetPosition(resourcesDataPosition);
      if (diesel::AreLoadParameters32Bit(version))
        writer.WriteType<uint32_t>(resourcesData);
      else
        writer.WriteType<uint64_t>(resourcesData);

      return true;
    }

    bool PackageBundle::open(Reader& outReader, unsigned int dbKey) {
      int headerIndex = -1;
      for (int i = 0; i < this->header.size(); i++) {
        if (this->header[i].first == dbKey) {
          headerIndex = i;
          break;
        }
      }
      if (headerIndex == -1)
        return false;

      OpenDataFile();

      outReader = this->fileContents;

      unsigned long long startPos = this->header[headerIndex].second;
      outReader.SetPosition(startPos);

      unsigned long long endPos = -1;

      if (headerIndex == this->header.size() - 1) {
        endPos = outReader.GetFileSize();
      }
      else {
        endPos = this->header[headerIndex + 1].second;
      }
      outReader.SetReplacementSize(endPos - startPos);

      return true;
    }
    size_t PackageBundle::GetFileSize(unsigned int dbKey) {
      int headerIndex = -1;
      for (int i = 0; i < this->header.size(); i++) {
        if (this->header[i].first == dbKey) {
          headerIndex = i;
          break;
        }
      }
      if (headerIndex == -1)
        return -1;

      size_t startPos = this->header[headerIndex].second;
      size_t endPos = -1;

      if (headerIndex == this->header.size() - 1) {
        this->OpenDataFile();

        endPos = this->fileContents.GetFileSize();

      }
      else {
        endPos = this->header[headerIndex + 1].second;
      }

      return endPos - startPos;
    }

    void PackageBundle::OpenDataFile() {
      if (this->fileContents.Valid())
        return;


      std::filesystem::path dataBundlePath = this->sourceFile.parent_path() / (diesel::ReplaceInString(this->sourceFile.filename().wstring(), L"_h", L""));

      Reader realDataFileReader;
      Reader dataFileReader(dataBundlePath);

      if (engineVersion.version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST || engineVersion.version == diesel::EngineVersion::PAYDAY_2_LEGACY_CONSOLE || engineVersion.version == diesel::EngineVersion::BIONIC_COMMANDO_REARMED2 || engineVersion.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
        dataFileReader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(engineVersion));
        dataFileReader.ReadCompressedDataStore(realDataFileReader);
      }
      else {
        realDataFileReader = dataFileReader;
      }

      this->fileContents = realDataFileReader;
    }

#pragma endregion

#pragma region Bundle

    Bundle::Bundle(const std::filesystem::path& basePath, const std::string& name, const DieselFormatsLoadingParameters& version) : Transport(version) {
      this->basePath = basePath;
      this->name = name;

      //assert(((version.sourcePlatform != diesel::FileSourcePlatform::NINTENDO_SWITCH && version.sourcePlatform != diesel::FileSourcePlatform::SONY_PLAYSTATION_4 && version.sourcePlatform != diesel::FileSourcePlatform::MICROSOFT_XBOX_ONE) && "Reading PAYDAY 2 Console bundles is unsupported due to annoying decompression problems"));

      std::vector<std::filesystem::path> bundleFilesToTryOpen;

      static const int NUM_BUNDLES = 50;

      if (version.version == diesel::EngineVersion::PAYDAY_THE_HEIST_V1) {
        for (int i = 0; i < NUM_BUNDLES; i++) {
          auto path = basePath / (name + "_" + std::to_string(i) + ".bundle");
          if (std::filesystem::exists(path))
            bundleFilesToTryOpen.push_back(path);
        }
      }
      else if (version.version == diesel::EngineVersion::PAYDAY_2_LEGACY_CONSOLE || version.version == diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST) {
        for (int i = 0; i < NUM_BUNDLES; i++) {
          auto path = basePath / (name + "_" + std::to_string(i) + "_h.bundle");
          if (std::filesystem::exists(path))
            bundleFilesToTryOpen.push_back(path);
        }
      }
      else if (version.version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST || version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
        for (int i = 0; i < NUM_BUNDLES; i++) {
          for(auto& bundleType : {"init", "default"}) {
            auto path = basePath / (std::string("stream_") + bundleType + "_" + std::to_string(i) + "_h.bundle");

            if (std::filesystem::exists(path))
              bundleFilesToTryOpen.push_back(path);
          }
        }
      }
      else if (version.version == diesel::EngineVersion::BIONIC_COMMANDO_REARMED2) {
        auto path = basePath / (name + ".bundle");

        if (std::filesystem::exists(path))
          bundleFilesToTryOpen.push_back(path);
      }


      for (const auto& bundleFile : bundleFilesToTryOpen) {
        std::filesystem::path fileContentsSource;

        if (version.version == diesel::EngineVersion::PAYDAY_THE_HEIST_V1 || version.version == diesel::EngineVersion::BIONIC_COMMANDO_REARMED2) {
          fileContentsSource = bundleFile;
        }
        else {
          fileContentsSource = diesel::ReplaceInString(bundleFile.wstring(), L"_h.bundle", L".bundle");
        }

        Reader reader;

        if (version.version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST || version.version == diesel::EngineVersion::PAYDAY_2_LEGACY_CONSOLE || version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
          Reader fileReader(bundleFile);
          fileReader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));
          fileReader.ReadCompressedDataStore(reader);
        }
        else {
          reader = Reader(bundleFile);
        }

        reader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));

        auto header_size = reader.ReadType<uint32_t>();

        if(version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE)
          reader.AddPosition(4);

        diesel::modern::SortMap<unsigned int, diesel::modern::Bundle::BundleEntry> header(reader, version);

        HeaderVectorType* header_vector = new HeaderVectorType();


        reader.SetPosition(header._data._data + 4);

        for (int i = 0; i < header._data._size; i++) {
          auto key = reader.ReadType<uint32_t>();

          auto offset = reader.ReadType<uint32_t>();
          auto size = reader.ReadType<uint32_t>();

          header_vector->push_back(std::make_pair(key, BundleEntry{.offset = offset, .size = size}));
        }

        HeaderData headerData = HeaderData();
        headerData.bundleFileContentsSource = fileContentsSource;

        this->_headers.push_back(header_vector);
        this->_archives.insert({ header_vector, headerData });

        assert(reader.ReadType<uint32_t>() == 0x94C51F19); // dsl::Bundle typeid (0x94C51F19)

        if (version.version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST || version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
          ///
          /// RAID: World War II multiplies unk1_size by 2, then does a for loop reading that number of uint32t's, for some reason.
          ///
          
          auto unk1_size = reader.ReadType<uint32_t>();

          for (int i = 0; i < unk1_size; i++) {
            auto dbKey = reader.ReadType<uint32_t>();
            auto uncompressedSize = reader.ReadType<uint32_t>();

            this->_raid_uncompressed_sizes.insert({ dbKey, uncompressedSize });
          }
        }
      }

      /*
      if (version.version == diesel::EngineVersion::PAYDAY_THE_HEIST_V1) { // dsl::Bundle::Bundle
        for (int i_bundle_file = 0; i_bundle_file < 50; i_bundle_file++) {
          auto filePath = basePath / (name + "_" + std::to_string(i_bundle_file) + ".bundle");

          if (!std::filesystem::exists(filePath))
            continue;

          Reader reader(filePath);
          reader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));

          auto header_size = reader.ReadType<uint32_t>();

          diesel::modern::SortMap<unsigned int, diesel::modern::Bundle::BundleEntry> header(reader, version);

          reader.SetPosition(header._data._data + 4);

          HeaderVectorType* header_vector = new HeaderVectorType();

          for (int i = 0; i < header._data._size; i++) {
            auto key = reader.ReadType<uint32_t>();

            auto offset = reader.ReadType<uint32_t>();
            auto size = reader.ReadType<uint32_t>();

            header_vector->push_back(std::make_pair(key, diesel::modern::Bundle::BundleEntry{ .offset = offset, .size = size }));
          }

          this->_pdth_headers.push_back(header_vector);

        }
      }
      else if (version.version == diesel::EngineVersion::PAYDAY_2_LEGACY_CONSOLE) {
        for (int i_bundle_file = 0; i_bundle_file < 50; i_bundle_file++) {
          auto filePath = basePath / (name + "_" + std::to_string(i_bundle_file) + "_h.bundle");

          if (!std::filesystem::exists(filePath))
            continue;

          Reader rawFileReader(filePath);
          rawFileReader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));

          Reader reader;

          rawFileReader.ReadCompressedDataStore(reader);
          reader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));

          auto header_size = reader.ReadType<uint32_t>();

          diesel::modern::SortMap<unsigned int, diesel::modern::Bundle::BundleEntry> header(reader, version);

          reader.SetPosition(header._data._data + 4);

          HeaderVectorType* header_vector = new HeaderVectorType();

          for (int i = 0; i < header._data._size; i++) {
            auto key = reader.ReadType<uint32_t>();

            auto offset = reader.ReadType<uint32_t>();
            auto size = reader.ReadType<uint32_t>();

            header_vector->push_back(std::make_pair(key, diesel::modern::Bundle::BundleEntry{ .offset = offset, .size = size }));
          }

          this->_pdth_headers.push_back(header_vector);

        }
      }
      else if (version.version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST) { // dsl::Bundle::Bundle from RAID: World War II. Function signature (as of U24.4): "\x4C\x89\x4C\x24\x00\x4C\x89\x44\x24\x00\x48\x89\x54\x24\x00\x48\x89\x4C\x24\x00\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xAC\x24" "xxxx?xxxx?xxxx?xxxx?xxxxxxxxxxxxxxxx" 
        // "init" and "default" are the only options
        
        // This format is also apparently used on the console versions of PAYDAY 2

        const std::vector<std::string> bundle_types = { "init", "default" };

        for (const std::string& bundle_type : bundle_types) {
          for (int i_bundle_file = 0; i_bundle_file < 50; i_bundle_file++) { // 50 chosen as pdth uses it
            auto path = basePath / (std::string("stream_") + bundle_type + "_" + std::to_string(i_bundle_file) + "_h.bundle");

            if (!std::filesystem::exists(path))
              continue;

            Reader reader;
            Reader reader2(path);
            reader2.ReadCompressedDataStore(reader);

            auto header_size = reader.ReadType<uint32_t>();
            diesel::modern::SortMap<unsigned int, diesel::modern::Bundle::BundleEntry> header(reader, version);

            HeaderVectorType* header_vector = new HeaderVectorType();


            reader.SetPosition(header._data._data + 4);

            for (int i = 0; i < header._data._size; i++) {
              auto key = reader.ReadType<uint32_t>();

              auto offset = reader.ReadType<uint32_t>();
              auto size = reader.ReadType<uint32_t>();

              header_vector->push_back(std::make_pair(key, diesel::modern::Bundle::BundleEntry{ .offset = offset, .size = size}));
            }

            reader.ReadType<uint32_t>(); // typeid

            ///
            /// RAID: World War II multiplies unk1_size by 2, then does a for loop reading that number of uint32t's, for some reason.
            ///
            
            auto unk1_size = reader.ReadType<uint32_t>(); // proper name unknown

            for (int i = 0; i < unk1_size; i++) {
              auto dbKey = reader.ReadType<uint32_t>();
              auto uncompressedSize = reader.ReadType<uint32_t>();

              this->_raid_uncompressed_sizes.insert({ dbKey, uncompressedSize });
            }

            if (bundle_type == "init")
              this->_raid_stream_init_headers.push_back(header_vector);
            else if (bundle_type == "default")
              this->_raid_stream_default_headers.push_back(header_vector);
            else
              __debugbreak();
          }
        }
      }
      else if (version.version == diesel::EngineVersion::BIONIC_COMMANDO_REARMED2) {
        if(!std::filesystem::exists(basePath / (name + ".bundle")))
          return;

        Reader reader(basePath / (name + ".bundle"));

        reader.SetSwapEndianness(diesel::AreLoadParametersForABigEndianPlatform(version));

        auto header_size = reader.ReadType<uint32_t>();

        diesel::modern::SortMap<unsigned int, diesel::modern::Bundle::BundleEntry> header(reader, version);

        reader.SetPosition(header._data._data + 4);

        HeaderVectorType* header_vector = new HeaderVectorType();

        for (int i = 0; i < header._data._size; i++) {
          auto key = reader.ReadType<uint32_t>();

          auto offset = reader.ReadType<uint32_t>();
          auto size = reader.ReadType<uint32_t>();

          header_vector->push_back(std::make_pair(key, diesel::modern::Bundle::BundleEntry{ .offset = offset, .size = size }));
        }

        this->_pdth_headers.push_back(header_vector);
      }
      */
    }

    Bundle::~Bundle() {
      for (auto header : this->_headers) {
        delete header;
      }
      _archives.clear();
    }

    bool Bundle::open(Reader& outReader, unsigned int dbKey) { // dsl::Bundle::open from PAYDAY: The Heist v1 and RAID: World War II (Function signature as of U24.4: "\x48\x89\x5C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x41\x8B\xD8" "xxxx?xxxxxxxxxxxxxx????xxx")
      for (auto header : _headers) {
        for (int i = 0; i < header->size(); i++) {
          auto& pair = (*header)[i];

          if (pair.first == dbKey) {
            auto& archive = _archives[header];

            if (!archive.bundleFileReader.Valid()) {
              archive.bundleFileReader = Reader(archive.bundleFileContentsSource);
            }
            Reader fileReader = archive.bundleFileReader;

            fileReader.SetPosition(pair.second.offset);
            fileReader.SetReplacementSize(pair.second.size);

            if ((engineVersion.version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST || engineVersion.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE) && this->_raid_uncompressed_sizes.contains(dbKey)) {
              unsigned int compressedSize = pair.second.size;
              char* compressed = new char[compressedSize];
              fileReader.ReadBytesToBuffer(compressed, compressedSize);

              unsigned int uncompressedSize = this->_raid_uncompressed_sizes[dbKey];
              char* uncompressed = new char[uncompressedSize];

              compression::ZlibDecompression::DecompressBuffer(compressed, compressedSize, uncompressed, uncompressedSize);

              delete[] compressed;

              outReader = Reader(uncompressed, uncompressedSize);
            }
            else {
              outReader = fileReader;
            }

            return true;
          }
        }
      }

      return false;

      /*

      for (int i = 0; i < this->_pdth_headers.size(); i++) {
        auto header = this->_pdth_headers[i];

        for (int j = 0; j < header->size(); j++) { // the game uses a binary search to index these
          auto& pair = (*header)[j];
          if (pair.first == dbKey) {
            auto filePath = basePath / (name + "_" + std::to_string(i) + ".bundle");
            if (this->engineVersion.version == diesel::EngineVersion::BIONIC_COMMANDO_REARMED2) {
              filePath = basePath / (name + ".bundle");
            }

            outReader = Reader(filePath);
            outReader.SetPosition(pair.second.offset);
            outReader.SetReplacementSize(pair.second.size);

            return true;
          }
        }
      }

      for (int i = 0; i < this->_raid_stream_default_headers.size() + this->_raid_stream_init_headers.size(); i++) { // RAID: World War II is assumed
        // Horrifying loop to make use of both lists in one block of code.

        bool isDefault = i < this->_raid_stream_default_headers.size();

        int bundleIndex = isDefault ? i : i - this->_raid_stream_default_headers.size();

        auto header = isDefault ? this->_raid_stream_default_headers[bundleIndex] : this->_raid_stream_init_headers[bundleIndex];

        for (int j = 0; j < header->size(); j++) {
          auto& pair = (*header)[j];
          if (pair.first == dbKey) {

            auto filePath = basePath / (std::string("stream_") + (isDefault ? "default" : "init") + "_" + std::to_string(bundleIndex) + ".bundle");

            Reader bundleReader(filePath);
            bundleReader.SetPosition(pair.second.offset);
            bundleReader.SetReplacementSize(pair.second.size);

            char* compressed = new char[pair.second.size];
            bundleReader.ReadBytesToBuffer(compressed, pair.second.size);

            unsigned int uncompressedSize = this->_raid_uncompressed_sizes[dbKey];
            char* uncompressed = new char[uncompressedSize];

            compression::ZlibDecompression::DecompressBuffer(compressed, pair.second.size, uncompressed, uncompressedSize);

            outReader = Reader(uncompressed, uncompressedSize);
            delete[] compressed;
          }
        }
      }
      return false;

      */
    }

#pragma endregion

#pragma region bundle_db.blb
    BundleDatabase::BundleDatabase() {
      this->_next_key = 0;
    }

    bool BundleDatabase::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
      if(version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE)
        reader.AddPosition(4);

      auto _properties = SortMap<Idstring, unsigned int>(reader, version);

      if(version.version == diesel::EngineVersion::PAYDAY_2_MODERN_CONSOLE)
        reader.AddPosition(20);
      auto _lookup = SortMap<DBExtKey, unsigned int>(reader, version);

      reader.SetPosition(_properties._data._data);

      for (int i = 0; i < _properties._data._size; i++) {
        auto key = reader.ReadType<uint64_t>();
        auto value = reader.ReadType<uint32_t>();

        reader.AddPosition(4);

        assert(key != 0);

        this->_properties.push_back(std::make_pair(Idstring(key), value));
      }

      reader.SetPosition(_lookup._data._data);

      this->_lookup.reserve(_lookup._data._size);
      for (int i = 0; i < _lookup._data._size; i++) {
        DBExtKey key{};

        key._type = reader.ReadType<uint64_t>();
        key._name = reader.ReadType<uint64_t>();
        key._properties = reader.ReadType<uint32_t>();


        assert(key._type != Idstring(0));
        assert(key._name != Idstring(0));

        reader.AddPosition(4); // likely alignment padding

        auto value = reader.ReadType<uint32_t>();

        reader.AddPosition(4); // likely alignment padding

        if (this->_next_key < value)
          _next_key = value;

        this->_lookup.push_back(std::make_pair(key, value));
      }

      reader.ReadType<uint32_t>(); // blob version

      _next_key++; // db keys start at 1

      return true;
    }

    void BundleDatabase::Write(Writer& writer, const DieselFormatsLoadingParameters& version) {
      uint64_t propertiesDataOffset = -1;
      SortMap<Idstring, unsigned int>::Write(writer, version, this->_properties.size(), this->_properties.size(), true, propertiesDataOffset);
      uint64_t lookupDataOffset = -1;
      SortMap<DBExtKey, unsigned int>::Write(writer, version, this->_lookup.size(), this->_lookup.size(), true, lookupDataOffset);

      writer.AddPosition(AreLoadParameters32Bit(version) ? 4 : 8); // padding

      auto propertiesData = writer.GetPosition();
      for (int i = 0; i < this->_properties.size(); i++) {
        writer.WriteType<uint64_t>(this->_properties[i].first);
        writer.WriteType<uint32_t>(this->_properties[i].second);

        writer.AddPosition(4);
      }

      auto lookupData = writer.GetPosition();

      for (int i = 0; i < this->_lookup.size(); i++) {
        writer.WriteType<uint64_t>(this->_lookup[i].first._type);
        writer.WriteType<uint64_t>(this->_lookup[i].first._name);

        writer.WriteType<uint32_t>(this->_lookup[i].first._properties);

        writer.AddPosition(4);

        writer.WriteType<uint32_t>(this->_lookup[i].second);

        writer.AddPosition(4);
      }

      writer.WriteType<uint32_t>(0xA0BEA7D9); // blob version

      writer.SetPosition(propertiesDataOffset);
      if (AreLoadParameters32Bit(version)) {
        writer.WriteType<uint32_t>(propertiesData);
      }
      else {
        writer.WriteType<uint64_t>(propertiesData);
      }
      writer.SetPosition(lookupDataOffset);
      if (AreLoadParameters32Bit(version)) {
        writer.WriteType<uint32_t>(lookupData);
      }
      else {
        writer.WriteType<uint64_t>(lookupData);
      }
    }

    void BundleDatabase::GetFileList(std::vector<ResourceID>& out) {
      for (auto& lookup : this->_lookup) {
        assert(lookup.first._type != Idstring(0));
        assert(lookup.first._name != Idstring(0));
        out.push_back(ResourceID{ lookup.first._type, lookup.first._name });
      }
    }
    DBExtKey BundleDatabase::GetLookupInformationFromDBKey(unsigned int key) {
      for (int i = 0; i < this->_lookup.size(); i++) {
        auto& entry = this->_lookup[i];
        if (entry.second == key)
          return entry.first;
      }
      return DBExtKey{ ._type = Idstring(-1), ._name = Idstring(-1), ._properties = (unsigned int)-1};
    }
    int BundleDatabase::GetDBKeyFromTypeAndName(const Idstring& type, const Idstring& name) {
      for (int i = 0; i < this->_lookup.size(); i++) {
        auto& entry = this->_lookup[i];
        if (entry.first._type == type && entry.first._name == name) {
          return entry.second;
        }
      }

      return -1;
    }

    const std::vector<std::pair<Idstring, unsigned int>>& BundleDatabase::GetProperties() {
      return this->_properties;
    }

    const std::vector<std::pair<DBExtKey, unsigned int>>& BundleDatabase::GetLookup() {
      return this->_lookup;
    }

    void BundleDatabase::AddFile(DBExtKey extKey, unsigned int dbKey) {
      this->_lookup.push_back(std::make_pair(extKey, dbKey));

      std::sort(this->_lookup.begin(), this->_lookup.end(), [](auto& a, auto& b) {
        return a.first < b.first;
        });

    }
#pragma endregion

  }
}
