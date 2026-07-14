#pragma once


#include <filesystem>
#include <string>
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

class Reader;

class WriterContainer {
protected:
  WriterContainer();
public:
  virtual ~WriterContainer();

  virtual void Close() = 0;

  virtual bool IsValid() const = 0;

  virtual unsigned long long WriteBytes(char* inBuffer, std::size_t size, unsigned long long position) = 0;
};

class MemoryWriterContainer : public WriterContainer {
public:
  MemoryWriterContainer();

  virtual void Close() override { closed = true; };

  virtual bool IsValid() const override { return true; }

  virtual unsigned long long WriteBytes(char* inBuffer, std::size_t size, unsigned long long position) override;

  const std::vector<char>& GetData() const { return data; }
  const std::vector<char>&& TakeData() { closed = true; return std::move(data); }

private:
  std::vector<char> data;
  bool closed;
};

class FileWriterContainer : public WriterContainer {
public:
  FileWriterContainer(const std::filesystem::path& path);
  ~FileWriterContainer();

  FileWriterContainer(const FileWriterContainer& other) = delete;
  FileWriterContainer& operator=(const FileWriterContainer& other) = delete;
  FileWriterContainer& operator=(FileWriterContainer& other) = delete;
public:
  virtual void Close() override;

  virtual bool IsValid() const override;

  virtual unsigned long long WriteBytes(char* inBuffer, std::size_t size, unsigned long long position) override;

private:
  HANDLE file;
};

class Writer {
public:
  // Initialise a writer to write to a memory buffer that the writer provides and controls
  Writer();
  // Initialise a writer from file
  Writer(const std::filesystem::path& path);
  ~Writer();

  Writer(const Writer& other);
  Writer& operator=(const Writer& other);

public:
  void Close();

  void AlignToSize(size_t bytes);

  unsigned long long GetPosition() const;
  void SetPosition(unsigned long long pos);

  // Adds a position to the writer. If the position is above the amount of written bytes the Win32 API will fill it with zeros.
  void AddPosition(long long pos);

  void SetSwapEndianness(bool swap);

  void WriteString(const std::string& str) { WriteBytes(str.data(), str.size() + 1); }

  void WriteBytes(const char* inBuffer, std::size_t size);

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

  WriterContainer* GetContainer() const { return container.get(); }

private:
  bool swapEndiannessOfIntegers;
  unsigned long long position;
  std::shared_ptr<WriterContainer> container;
  //HANDLE file;
};