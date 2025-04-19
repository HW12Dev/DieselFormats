#pragma once

#include "fileio/reader.h"
#include "fileio/writer.h"
#include "diesel/modern/modern_shared.h"
#include "diesel/modern/hash.h"

#include <vector>
#include <unordered_map>
#include <filesystem>

namespace diesel {
  namespace modern {
    // dsl::PackageBundle::<unnamed_tag>
    const unsigned int TypeId_PackageBundle = 0x19b7461f;
    const unsigned int TypeId_BundleHeader = 0xebfcc5f8;


    struct DBExtKey {
      Idstring _type;
      Idstring _name;
      unsigned int _properties;

      bool operator<(const DBExtKey& other);
    };

    struct ResourceID {
      Idstring type;
      Idstring name;

      bool operator<(const ResourceID& other) const;
    };

    class Transport {
    public:
      Transport(const DieselFormatsLoadingParameters& version) : engineVersion(version){}
      virtual ~Transport() = default;
      virtual bool open(Reader& outReader, unsigned int dbKey) = 0;

    protected:
      const DieselFormatsLoadingParameters& engineVersion;
      std::filesystem::path basePath;
    };

    namespace blobtypes {
      class Bundle {};
      class PackageBundle : public Transport {
      public:
        PackageBundle(const std::filesystem::path& source, Reader& reader, const DieselFormatsLoadingParameters& version);

      public:
        virtual bool open(Reader& outReader, unsigned int dbKey);

        const std::vector<diesel::modern::ResourceID>& GetResources();
      private:
        Reader fileContents;
        std::filesystem::path sourceFile;

        std::vector<std::pair<unsigned int, unsigned int>> header;
        std::vector<diesel::modern::ResourceID> resources;
      };
    }


    class MultiFileTransport : public Transport { // used in production for asset packaging but is still available to use in release builds of diesel games.
    public:
      MultiFileTransport(const std::filesystem::path& basePath, const DieselFormatsLoadingParameters& version);
    public:
      virtual bool open(Reader& outReader, unsigned int dbKey);

    private:
    };

    class Bundle : public Transport {
    public:
      struct BundleEntry {
        uint64_t offset;
        uint64_t size;
      };

      typedef std::vector<std::pair<unsigned int, diesel::modern::Bundle::BundleEntry>> HeaderVectorType;
    public:
      Bundle(const std::filesystem::path& basePath, const std::string& name, const DieselFormatsLoadingParameters& version);
      ~Bundle();
    public:
      virtual bool open(Reader& outReader, unsigned int dbKey);

    private:
      std::string name;
      // PAYDAY: The Heist
      std::vector<HeaderVectorType*> _pdth_headers;

      // RAID: World War II
      // NOTE: RAID very likely stores them in one vector instead of two
      std::vector<HeaderVectorType*> _raid_stream_default_headers;
      std::vector<HeaderVectorType*> _raid_stream_init_headers;
      std::unordered_map<unsigned int, unsigned int> _raid_uncompressed_sizes;
    };

    class BundleDatabase { // proper name for the "bundle_db.blb" format is "Diesel Bundle Database" (thanks RAID Modding Tools)
    public:
      BundleDatabase(Reader& reader, const DieselFormatsLoadingParameters& version);


      void Write(Writer& writer, const DieselFormatsLoadingParameters& version);
    public:
      void GetFileList(std::vector<ResourceID>& out);

      DBExtKey GetLookupInformationFromDBKey(unsigned int key);
      int GetDBKeyFromTypeAndName(const Idstring& type, const Idstring& name);

      const std::vector<std::pair<Idstring, unsigned int>>& GetProperties();

      const std::vector<std::pair<DBExtKey, unsigned int>>& GetLookup();

      // For advanced users only, do not call this if you do not know what it does
      void AddFile(DBExtKey extKey, unsigned int dbKey);

    private:
      std::vector<std::pair<Idstring, unsigned int>> _properties;
      std::vector<std::pair<DBExtKey, unsigned int>> _lookup;

      unsigned int _next_key;
    };
  }
}