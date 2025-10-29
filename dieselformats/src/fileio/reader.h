#pragma once

#include <filesystem>
#include <string>
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

class ReaderContainer {
protected:
  ReaderContainer();
public:
  virtual ~ReaderContainer();

  virtual void Close() = 0;

  virtual unsigned long long GetFileSize() = 0;
  
  virtual unsigned long long ReadBytesToBuffer(char* outBuffer, std::size_t size, unsigned long long position) = 0;

protected:
  unsigned long long size;
};

class MemoryReaderContainer : public ReaderContainer {
public:
  MemoryReaderContainer(char* buffer, std::size_t size);
  ~MemoryReaderContainer();
public:
  virtual void Close();

  virtual unsigned long long GetFileSize() override;

  virtual unsigned long long ReadBytesToBuffer(char* outBuffer, std::size_t size, unsigned long long position) override;

protected:
  char* data;
};
class FileReaderContainer : public ReaderContainer {
public:
  FileReaderContainer(const std::filesystem::path& path);
  ~FileReaderContainer();
public:
  virtual void Close();

  virtual unsigned long long GetFileSize() override;

  virtual unsigned long long ReadBytesToBuffer(char* outBuffer, std::size_t size, unsigned long long position) override;

protected:
  HANDLE file;
  std::filesystem::path sourcePath;
};

class Reader {
public:
  Reader();
  Reader(const std::filesystem::path& path);
  Reader(char* buffer, std::size_t size);
  ~Reader();

  Reader(const Reader& other);
  Reader& operator=(const Reader& other);

public:
  unsigned long long GetPosition() const;
  void SetPosition(unsigned long long pos);
  void AddPosition(long long addPos);

  void SetReplacementSize(unsigned long long size);
  void SetSwapEndianness(bool swap);

  bool AtEndOfBuffer();

  void Close();

  unsigned long long GetFileSize();

  void ReadBytesToBuffer(char* outBuffer, std::size_t size);
  template <typename T>
  void ReadBytesToBuffer(T* outBuffer, std::size_t size) {
    return ReadBytesToBuffer((char*)outBuffer, size);
  }

  template<typename T> T ReadType() {
    T value{};

    ReadBytesToBuffer(&value, sizeof(T));

    return value;
  }
  template<> uint16_t ReadType() { uint16_t value{}; ReadBytesToBuffer(&value, sizeof(uint16_t)); if (this->swapEndiannessOfIntegers) value = _byteswap_ushort(value); return value; }
  template<> uint32_t ReadType() { uint32_t value{}; ReadBytesToBuffer(&value, sizeof(uint32_t)); if (this->swapEndiannessOfIntegers) value = _byteswap_ulong(value); return value; }
  template<> uint64_t ReadType() { uint64_t value{}; ReadBytesToBuffer(&value, sizeof(uint64_t)); if (this->swapEndiannessOfIntegers) value = _byteswap_uint64(value); return value; }
  template<> int16_t ReadType() { int16_t value{}; ReadBytesToBuffer(&value, sizeof(int16_t)); if (this->swapEndiannessOfIntegers) value = _byteswap_ushort(value); return value; }
  template<> int32_t ReadType() { int32_t value{}; ReadBytesToBuffer(&value, sizeof(int32_t)); if (this->swapEndiannessOfIntegers) value = _byteswap_ulong(value); return value; }
  template<> int64_t ReadType() { int64_t value{}; ReadBytesToBuffer(&value, sizeof(int64_t)); if (this->swapEndiannessOfIntegers) value = _byteswap_uint64(value); return value; }

  /// <summary>
  /// Reads a null terminated string starting at the current position in the buffer.
  /// </summary>
  std::string ReadString();

  /// <summary>
  /// Reads a string with the length provided as a uint32_t at the current position in the buffer.
  /// </summary>
  std::string ReadLengthPrefixedString();

  /// <summary>
  /// Read's a modern Diesel formatted zlib compressed buffer from the input file into memory. (modern diesel format is "{uncompressedSize:u64}{compressedBufferSize:u32}{compressedBuffer:char[compressedBufferSize]}"
  /// Endian swaps aren't copied from the input reader to the output reader
  /// </summary>
  /// <param name="outReader"></param>
  void ReadCompressedDataStore(Reader& outReader);

  /// <summary>
  /// Read's a BANDITS - Phoenix Rising .enc file to the provided buffer.
  /// This does not include a null terminator.
  /// 
  /// More information about BANDITS' encryption implementation can be found in the implementation of this function.
  /// </summary>
  /// <param name="outReader"></param>
  void ReadBANDITSEncryptedFile(Reader& outReader);

  bool Valid() const;
private:
  bool isFileBased;
  std::shared_ptr<ReaderContainer> container;
  unsigned long long position;

  unsigned long long replacementSize; // optional size parameter, that can be set by diesel file loading functions to be the current size of the contained file

  bool swapEndiannessOfIntegers;
};

//using FileReader = Reader;