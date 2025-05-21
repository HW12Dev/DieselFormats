#pragma once


#include <filesystem>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

class Reader;
class Writer {
public:
  Writer(const std::filesystem::path& path);

public:
  void Close();


  unsigned long long GetPosition() const;
  void SetPosition(unsigned long long pos);

  // Adds a position to the writer. If the position is above the amount of written bytes the Win32 API will fill it with zeros.
  void AddPosition(long long pos);

  void SetSwapEndianness(bool swap);

  void WriteBytes(char* inBuffer, std::size_t size);

  // write the remaining contents of a reader object
  void WriteReader(Reader& reader);

  void WriteReaderToCompressedDataStore(Reader& reader);

  template<typename T> void WriteType(const T& value) {
    this->WriteBytes((char*)&value, sizeof(T));
  }

  template<> void WriteType(const uint16_t& value) { uint16_t writeValue = value; if (this->swapEndiannessOfIntegers) writeValue = _byteswap_ushort(value); WriteBytes((char*)&writeValue, sizeof(uint16_t)); }
  template<> void WriteType(const uint32_t& value) { uint32_t writeValue = value; if (this->swapEndiannessOfIntegers) writeValue = _byteswap_ulong(value); WriteBytes((char*)&writeValue, sizeof(uint32_t)); }
  template<> void WriteType(const uint64_t& value) { uint64_t writeValue = value; if (this->swapEndiannessOfIntegers) writeValue = _byteswap_uint64(value); WriteBytes((char*)&writeValue, sizeof(uint64_t)); }
  template<> void WriteType(const int16_t& value) { int16_t writeValue = value; if (this->swapEndiannessOfIntegers) writeValue = _byteswap_ushort(value); WriteBytes((char*)&writeValue, sizeof(int16_t)); }
  template<> void WriteType(const int32_t& value) { int32_t writeValue = value; if (this->swapEndiannessOfIntegers) writeValue = _byteswap_ulong(value); WriteBytes((char*)&writeValue, sizeof(int32_t)); }
  template<> void WriteType(const int64_t& value) { int64_t writeValue = value; if (this->swapEndiannessOfIntegers) writeValue = _byteswap_uint64(value); WriteBytes((char*)&writeValue, sizeof(int64_t)); }


private:
  bool swapEndiannessOfIntegers;
  unsigned long long position;
  HANDLE file;
};