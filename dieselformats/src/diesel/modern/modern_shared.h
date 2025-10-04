#pragma once

#include <string>
#include <cassert>

#include "fileio/reader.h"
#include "fileio/writer.h"
#include "diesel/shared.h"

namespace diesel {
  namespace modern {

    std::string hex(const char* bytes, int n);

    class BlobSaverChunk {
    public:
      BlobSaverChunk(Reader& reader, const DieselFormatsLoadingParameters& version);
    public:
      unsigned long long _size;
      unsigned long long _data;
    };
    class String {
    public:
      String(Reader& reader, const DieselFormatsLoadingParameters& version);

      static void Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t& outPositionOfStringPointerInBuffer);
    public:
      unsigned long long _s;
    };

    template<typename T>
    class Vector {
    public:
      Vector() = default;
      Vector(Reader& reader, const DieselFormatsLoadingParameters& version);

      static void Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t size, uint64_t capacity, uint64_t& outPositionOfDataPointerInBuffer);

    public:
      unsigned long long _size;
      unsigned long long _capacity;
      unsigned long long _data; // for blobs: offset from the beginning of the buffer to the start of where the data pointer normally points to. e.g. T* real_data = (buffer_start + _data)
    };
    template<typename Key, typename Value>
    class Pair {};
    template<typename Key, typename Value>
    class SortMap {
    public:
      SortMap() = default;
      SortMap(Reader& reader, const DieselFormatsLoadingParameters& version);

      static void Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t size, uint64_t capacity, bool is_sorted, uint64_t& outPositionOfDataPointerInBuffer);

    public:
      Vector<Pair<Key, Value>> _data;
      bool _is_sorted;
    };

    

    template<typename T>
    Vector<T>::Vector(Reader& reader, const DieselFormatsLoadingParameters& version) {
      if (version.version == EngineVersion::PAYDAY_2_LINUX_LATEST) { // PAYDAY 2 Linux stores everything except pointers the same way as 32bit PAYDAY 2 for windows
        this->_size = (unsigned long long)reader.ReadType<uint32_t>();
        this->_capacity = (unsigned long long)reader.ReadType<uint32_t>();
        this->_data = (unsigned long long)reader.ReadType<uint64_t>();
        reader.AddPosition(8); // _allocator
      }
      else if(version.version == EngineVersion::RAID_WORLD_WAR_II_LATEST) { // 64 bit engine version (RAID: World War II)
        this->_size = reader.ReadType<uint64_t>();
        this->_capacity = reader.ReadType<uint64_t>();
        this->_data = reader.ReadType<uint64_t>();
        reader.AddPosition(8); // _allocator
      }
      else if (version.version == EngineVersion::PAYDAY_2_MODERN_CONSOLE) {
        this->_size = (unsigned long long)reader.ReadType<uint32_t>();
        this->_capacity = (unsigned long long)reader.ReadType<uint32_t>();
        this->_data = (unsigned long long)reader.ReadType<uint32_t>();
      }
      else if (version.version == EngineVersion::PAYDAY_2_LEGACY) {
        // this version doesn't have an _allocator field serialised

        this->_size = (unsigned long long)reader.ReadType<uint32_t>();
        this->_capacity = (unsigned long long)reader.ReadType<uint32_t>();
        this->_data = (unsigned long long)reader.ReadType<uint32_t>();
      }
      else if (AreLoadParameters32Bit(version)) { // 32 bit engine version (PAYDAY: The Heist, PAYDAY 2)
        this->_size = (unsigned long long)reader.ReadType<uint32_t>();
        this->_capacity = (unsigned long long)reader.ReadType<uint32_t>();
        this->_data = (unsigned long long)reader.ReadType<uint32_t>();
        reader.AddPosition(4); // _allocator
      }
      else {
        throw std::runtime_error("Engine version is not 32bit, but isn't RAID or PD2 Linux");
      }
    }

    template<typename T>
    void Vector<T>::Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t size, uint64_t capacity, uint64_t& outPositionOfDataPointerInBuffer) {
      // TODO: strange PAYDAY_2_LEGACY data
      if (AreLoadParameters32Bit(version)) {
        writer.WriteType<uint32_t>(size);
        writer.WriteType<uint32_t>(capacity);

        outPositionOfDataPointerInBuffer = writer.GetPosition();
        writer.WriteType<uint32_t>(0);

        writer.AddPosition(4); // _allocator
      }
      else {
        writer.WriteType<uint64_t>(size);
        writer.WriteType<uint64_t>(capacity);

        outPositionOfDataPointerInBuffer = writer.GetPosition();
        writer.WriteType<uint64_t>(0);

        
        writer.AddPosition(8); // _allocator
      }
    }

    template<typename Key, typename Value>
    SortMap<Key, Value>::SortMap(Reader& reader, const DieselFormatsLoadingParameters& version) {
      if (AreLoadParameters32Bit(version)) {
        reader.AddPosition(4); // _less
      }
      else {
        reader.AddPosition(8); // _less
      }
      this->_data = Vector<Pair<Key, Value>>(reader, version);

      if(version.version == diesel::EngineVersion::PAYDAY_2_LEGACY)
        reader.AddPosition(4);

      this->_is_sorted = (bool)reader.ReadType<uint8_t>();

      if (AreLoadParameters32Bit(version)) { // class alignment shenanigans
        reader.AddPosition(3);
      }
      else {
        reader.AddPosition(7);
      }
    }

    template<typename Key, typename Value>
    void SortMap<Key, Value>::Write(Writer& writer, const DieselFormatsLoadingParameters& version, uint64_t size, uint64_t capacity, bool is_sorted, uint64_t& outPositionOfDataPointerInBuffer) {
      if (AreLoadParameters32Bit(version)) {
        writer.AddPosition(4);
      }
      else {
        writer.AddPosition(8);
      }

      Vector<Pair<Key, Value>>::Write(writer, version, size, capacity, outPositionOfDataPointerInBuffer);

      // TODO: strange PAYDAY_2_LEGACY data

      writer.WriteType<uint8_t>(is_sorted);

      if (AreLoadParameters32Bit(version)) { // class alignment
        writer.AddPosition(3);
      }
      else {
        writer.AddPosition(7);
      }
    }
  }
}