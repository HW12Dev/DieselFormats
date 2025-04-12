#pragma once

#include <string>

#include "fileio/reader.h"
#include "fileio/writer.h"
#include "diesel/shared.h"

namespace diesel {
  namespace modern {
    enum class ModernEngineVersion : EngineVersionBaseType {
      INVALID_NOT_MODERN = 0, // used for conversion between the generic EngineVersion enum
      PAYDAY_THE_HEIST_V1 = 1,
      PAYDAY_THE_HEIST_LATEST,
      PAYDAY_2_LATEST,
      PAYDAY_2_XB1_PS4, // XB1, PS4 and Switch might all share 1 base engine version, but they are seperate for now
      PAYDAY_2_SWITCH,
      PAYDAY_2_LINUX_LATEST,
      RAID_WORLD_WAR_II_LATEST
    };

    ModernEngineVersion ToModernVersion(diesel::EngineVersion version);
    diesel::EngineVersion ToGenericVersion(ModernEngineVersion version);

    bool IsEngineVersion32Bit(ModernEngineVersion version);

    std::string hex(const char* bytes, int n);
    
    class String {
    public:
      String(Reader& reader, ModernEngineVersion version);

    public:
      unsigned long long _s;
    };

    template<typename T>
    class Vector {
    public:
      Vector() = default;
      Vector(Reader& reader, ModernEngineVersion version);

      static void Write(Writer& writer, ModernEngineVersion version, uint64_t size, uint64_t capacity, uint64_t& outPositionOfDataPointerInBuffer);

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
      SortMap(Reader& reader, ModernEngineVersion version);

      static void Write(Writer& writer, ModernEngineVersion version, uint64_t size, uint64_t capacity, bool is_sorted, uint64_t& outPositionOfDataPointerInBuffer);

    public:
      Vector<Pair<Key, Value>> _data;
      bool _is_sorted;
    };

    

    template<typename T>
    Vector<T>::Vector(Reader& reader, ModernEngineVersion version) {
      if (IsEngineVersion32Bit(version)) {
        this->_size = (unsigned long long)reader.ReadType<uint32_t>();
        this->_capacity = (unsigned long long)reader.ReadType<uint32_t>();
        this->_data = (unsigned long long)reader.ReadType<uint32_t>();
        reader.AddPosition(4); // _allocator
      }
      else {
        this->_size = reader.ReadType<uint64_t>();
        this->_capacity = reader.ReadType<uint64_t>();
        this->_data = reader.ReadType<uint64_t>();
        reader.AddPosition(8); // _allocator
      }
    }

    template<typename T>
    void Vector<T>::Write(Writer& writer, ModernEngineVersion version, uint64_t size, uint64_t capacity, uint64_t& outPositionOfDataPointerInBuffer) {
      if (IsEngineVersion32Bit(version)) {
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
    SortMap<Key, Value>::SortMap(Reader& reader, ModernEngineVersion version) {
      if (IsEngineVersion32Bit(version)) {
        reader.AddPosition(4); // _less
      }
      else {
        reader.AddPosition(8); // _less
      }
      this->_data = Vector<Pair<Key, Value>>(reader, version);

      this->_is_sorted = (bool)reader.ReadType<uint8_t>();

      if (IsEngineVersion32Bit(version)) { // class alignment shenanigans
        reader.AddPosition(3);
      }
      else {
        reader.AddPosition(7);
      }
    }

    template<typename Key, typename Value>
    void SortMap<Key, Value>::Write(Writer& writer, ModernEngineVersion version, uint64_t size, uint64_t capacity, bool is_sorted, uint64_t& outPositionOfDataPointerInBuffer) {
      if (IsEngineVersion32Bit(version)) {
        writer.AddPosition(4);
      }
      else {
        writer.AddPosition(8);
      }

      Vector<Pair<Key, Value>>::Write(writer, version, size, capacity, outPositionOfDataPointerInBuffer);

      writer.WriteType<uint8_t>(is_sorted);

      if (IsEngineVersion32Bit(version)) { // class alignment
        writer.AddPosition(3);
      }
      else {
        writer.AddPosition(7);
      }
    }
  }
}
bool operator==(const diesel::modern::ModernEngineVersion a, const diesel::modern::ModernEngineVersion b);
bool operator!=(const diesel::modern::ModernEngineVersion a, const diesel::modern::ModernEngineVersion b);