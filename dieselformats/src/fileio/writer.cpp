#include "writer.h"

#include "reader.h"

#include "fileio/zlibcompression.h"

Writer::Writer(const std::filesystem::path& path) {

  this->position = 0;

  this->file = INVALID_HANDLE_VALUE;

  this->file = CreateFileW(path.wstring().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

void Writer::Close() {
  if (this->file != INVALID_HANDLE_VALUE)
    CloseHandle(file);
}

unsigned long long Writer::GetPosition() const {
  return this->position;
}

void Writer::SetPosition(unsigned long long pos) {
  this->position = pos;
}

void Writer::AddPosition(long long pos) {
  this->position += pos;
}

void Writer::SetSwapEndianness(bool swap) {
  this->swapEndiannessOfIntegers = swap;
}

void Writer::WriteBytes(char* inBuffer, std::size_t size) {

  DWORD bytesWritten{};
  LARGE_INTEGER li{};


  SetFilePointerEx(this->file, LARGE_INTEGER{.QuadPart = (int64_t)this->position}, & li, FILE_BEGIN);
  WriteFile(this->file, inBuffer, size, &bytesWritten, NULL);

  this->position += size;
}

void Writer::WriteReader(Reader& reader) {
  //auto size = reader.GetFileSize() - reader.GetPosition(); // Line causes crashes if reading a file stream with replacement size set and position is greater than replacement size.
  auto size = reader.GetFileSize();

  char* buffer = new char[size];
  reader.ReadBytesToBuffer(buffer, size);

  this->WriteBytes(buffer, size);

  delete[] buffer;
}

void Writer::WriteReaderToCompressedDataStore(Reader& reader) {
  auto uncompressedTotalSize = reader.GetFileSize();

  this->WriteType<uint64_t>(uncompressedTotalSize);

  unsigned int chunks = (unsigned int)(uncompressedTotalSize / Diesel_CompressedDataStore_ChunkSize);

  char* uncompressedChunkDataBuffer = new char[Diesel_CompressedDataStore_ChunkSize] {};
  char* compressedChunkDataBuffer = new char[Diesel_CompressedDataStore_ChunkSize * 2] {}; // Diesel reserves 0x20000

  auto inputStartPosition = reader.GetPosition();

  for (unsigned int i = 0; i < chunks; i++) {
    reader.ReadBytesToBuffer(uncompressedChunkDataBuffer, Diesel_CompressedDataStore_ChunkSize);

    std::size_t compressedSize = compression::ZlibDecompression::CompressBuffer(uncompressedChunkDataBuffer, Diesel_CompressedDataStore_ChunkSize, compressedChunkDataBuffer, Diesel_CompressedDataStore_ChunkSize * 2, ZLIB_COMPRESSION_LEVEL_DEFAULT_COMPRESSION); // Diesel uses default compression by default

    if (compressedSize < Diesel_CompressedDataStore_ChunkSize) {
      this->WriteType<uint32_t>(compressedSize);
      this->WriteBytes(compressedChunkDataBuffer, compressedSize);
    }
    else {
      this->WriteType<uint32_t>(Diesel_CompressedDataStore_ChunkSize);
      this->WriteBytes(uncompressedChunkDataBuffer, Diesel_CompressedDataStore_ChunkSize);
    }
  }

  memset(uncompressedChunkDataBuffer, 0, Diesel_CompressedDataStore_ChunkSize);
  memset(compressedChunkDataBuffer, 0, Diesel_CompressedDataStore_ChunkSize * 2);

  auto remainingBytesInInput = reader.GetFileSize() - (reader.GetPosition() - inputStartPosition);

  reader.ReadBytesToBuffer(uncompressedChunkDataBuffer, remainingBytesInInput);
  std::size_t compressedSize = compression::ZlibDecompression::CompressBuffer(uncompressedChunkDataBuffer, remainingBytesInInput, compressedChunkDataBuffer, Diesel_CompressedDataStore_ChunkSize * 2, ZLIB_COMPRESSION_LEVEL_DEFAULT_COMPRESSION);

  if (compressedSize < Diesel_CompressedDataStore_ChunkSize) {
    this->WriteType<uint32_t>(compressedSize);
    this->WriteBytes(compressedChunkDataBuffer, compressedSize);
  }
  else {
    this->WriteType<uint32_t>(remainingBytesInInput);
    this->WriteBytes(uncompressedChunkDataBuffer, remainingBytesInInput);
  }

  delete[] uncompressedChunkDataBuffer;
  delete[] compressedChunkDataBuffer;
}
