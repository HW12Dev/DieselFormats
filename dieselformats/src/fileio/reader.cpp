#include "fileio/reader.h"

#include "fileio/zlibcompression.h"

#include <cassert>
#include <unordered_map>

#pragma region Containers
ReaderContainer::ReaderContainer() { this->size = -1; }
ReaderContainer::~ReaderContainer() {}

FileReaderContainer::FileReaderContainer(const std::filesystem::path& path) {
  this->file = INVALID_HANDLE_VALUE;
  this->size = -1;

  this->file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  this->sourcePath = path;

  if (this->file == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("File: " + path.string() + " does not exist.");
  }
}

FileReaderContainer::~FileReaderContainer() {
  if (this->file != INVALID_HANDLE_VALUE)
    CloseHandle(this->file);
}

void FileReaderContainer::Close() {
  if(this->file != INVALID_HANDLE_VALUE)
    CloseHandle(this->file);
  this->file = INVALID_HANDLE_VALUE;
}

unsigned long long FileReaderContainer::GetFileSize() {
  if (this->size != -1)
    return size;

  LARGE_INTEGER sizeLI;
  GetFileSizeEx(this->file, &sizeLI);

  this->size = (unsigned long long)sizeLI.QuadPart;

  return this->size;
}

unsigned long long FileReaderContainer::ReadBytesToBuffer(char* outBuffer, std::size_t size, unsigned long long position) {
  //OVERLAPPED overlapped{};
  //overlapped.Offset = LOWORD(this->position);
  //overlapped.OffsetHigh = HIWORD(this->position);

  if (size + position > this->GetFileSize()) {
    throw std::runtime_error("FileReaderContainer read beyond its size (" + this->sourcePath.string() + ")");
    return 0;
  }

  DWORD bytesRead{};
  //ReadFile(this->file, outBuffer, (DWORD)size, &bytesRead, &overlapped);

  LARGE_INTEGER li{};
  SetFilePointerEx(this->file, LARGE_INTEGER{.QuadPart = (LONGLONG)position}, &li, FILE_BEGIN);
  assert(li.QuadPart == position);

  auto result = ReadFile(this->file, outBuffer, (DWORD)size, &bytesRead, NULL);

  assert(result == TRUE);

  return size;
}

MemoryReaderContainer::MemoryReaderContainer(char* buffer, std::size_t size) {
  this->size = size;

  this->data = buffer;
}

MemoryReaderContainer::~MemoryReaderContainer() {
  if (this->data) {
    delete[] this->data;
    this->data = nullptr;
  }
}

void MemoryReaderContainer::Close() {
  if (this->data) {
    delete[] this->data;
    this->data = nullptr;
  }
}

unsigned long long MemoryReaderContainer::GetFileSize() {
  return this->size;
}

unsigned long long MemoryReaderContainer::ReadBytesToBuffer(char* outBuffer, std::size_t size, unsigned long long position) {
  if (position + size > this->GetFileSize()) {
    throw std::runtime_error("MemoryReaderContainer read beyond its size");
    memset(outBuffer, 0, size);
    return 0;
  }

  memcpy(outBuffer, &this->data[position], size);

  return size;
}

#pragma endregion

Reader::Reader() {
  this->isFileBased = false;
  this->container = nullptr;
  this->position = -1;
  this->replacementSize = -1;
  this->swapEndiannessOfIntegers = false;
}

Reader::Reader(const std::filesystem::path& path) {
  this->isFileBased = true;
  this->container = std::make_shared<FileReaderContainer>(path);
  this->position = 0;
  this->replacementSize = -1;
  this->swapEndiannessOfIntegers = false;
}

Reader::Reader(char* buffer, std::size_t size) {
  this->isFileBased = false;
  this->container = std::make_shared<MemoryReaderContainer>(buffer, size);
  this->position = 0;
  this->replacementSize = -1;
  this->swapEndiannessOfIntegers = false;
}

Reader::~Reader() {
}

Reader::Reader(const Reader& other) {
  *this = other;
}

Reader& Reader::operator=(const Reader& other) {
  this->isFileBased = other.isFileBased;
  this->container = other.container;
  this->position = other.position;
  this->replacementSize = other.replacementSize;
  this->swapEndiannessOfIntegers = other.swapEndiannessOfIntegers;

  return *this;
}

unsigned long long Reader::GetPosition() const {
  return this->position;
}

void Reader::SetPosition(unsigned long long pos) {
  this->position = pos;
}

void Reader::AddPosition(long long addPos) {
  this->position += addPos;
}

void Reader::SetReplacementSize(unsigned long long size) {
  this->replacementSize = size;
}

void Reader::SetSwapEndianness(bool swap) {
  this->swapEndiannessOfIntegers = swap;
}

bool Reader::AtEndOfBuffer() {
  return this->position >= this->GetFileSize();
}

void Reader::Close() {
  if (this->container && this->container.use_count() == 1)
    this->container->Close();
}

unsigned long long Reader::GetFileSize() {
  if (this->replacementSize != -1)
    return this->replacementSize;

  return this->container->GetFileSize();
}

void Reader::ReadBytesToBuffer(char* outBuffer, std::size_t size) {
  this->position += this->container->ReadBytesToBuffer(outBuffer, size, this->position);
}

void Reader::ReadCompressedDataStore(Reader& outReader) {
  auto uncompressedTotalSize = ReadType<uint64_t>(); // could be uint32_t on 32bit diesel
  char* uncompressed = new char[uncompressedTotalSize] {};
  std::size_t uncompressedPosition = 0;

  auto inputStartPosition = GetPosition();

  unsigned int chunks = (unsigned int)(uncompressedTotalSize / Diesel_CompressedDataStore_ChunkSize);

  char* compressedChunkDataBuffer = new char[Diesel_CompressedDataStore_ChunkSize] {};

  for (unsigned int i = 0; i < chunks; i++) {
    uint32_t chunkSize = ReadType<uint32_t>();

    if (chunkSize < Diesel_CompressedDataStore_ChunkSize) { // compressed
      ReadBytesToBuffer(compressedChunkDataBuffer, chunkSize);

      compression::ZlibDecompression::DecompressBuffer(compressedChunkDataBuffer, chunkSize, uncompressed + uncompressedPosition, uncompressedTotalSize - uncompressedPosition);
      uncompressedPosition += Diesel_CompressedDataStore_ChunkSize;
    }
    else { // uncompressed
      ReadBytesToBuffer(uncompressed + uncompressedPosition, chunkSize);
      uncompressedPosition += chunkSize;
    }
  }

  //memset(compressedChunkDataBuffer, 0, Diesel_CompressedDataStore_ChunkSize);

  if (uncompressedPosition != uncompressedTotalSize) { // compressed chunk remaining
    uint32_t chunkSize = ReadType<uint32_t>();

    if (chunkSize < Diesel_CompressedDataStore_ChunkSize) { // compressed
      ReadBytesToBuffer(compressedChunkDataBuffer, chunkSize);

      compression::ZlibDecompression::DecompressBuffer(compressedChunkDataBuffer, chunkSize, uncompressed + uncompressedPosition, uncompressedTotalSize - uncompressedPosition);
      uncompressedPosition += (uncompressedTotalSize - uncompressedPosition);
    }
    else { // uncompressed
      ReadBytesToBuffer(uncompressed + uncompressedPosition, chunkSize);
      uncompressedPosition += chunkSize;
    }
  }

  delete[] compressedChunkDataBuffer;

  outReader = Reader(uncompressed, uncompressedTotalSize);
}

void Reader::ReadWithXORKey(Reader& outReader, const char* key, size_t keyLength) {
  auto fileSize = this->GetFileSize();
  char* data = new char[fileSize];
  this->ReadBytesToBuffer(data, fileSize);

  int keyIndex = 0;

  for (int i = 0; i < fileSize; i++) {
    data[i] = data[i] ^ (char)key[keyIndex % keyLength];
    keyIndex++;
  }

  outReader = Reader(data, fileSize);
}

void Reader::ReadWithXORKeyTransformInPlace(Reader& outReader, const char* key, size_t keyLength)
{
  auto fileSize = this->GetFileSize();
  char* data = new char[fileSize];
  this->ReadBytesToBuffer(data, fileSize);

  int keyIndex = 0;

  for (int i = 0; i < fileSize; i++) {
    int keyIndex = ((fileSize + i) * 7) % keyLength;
    data[i] = data[i] ^ (char)(key[keyIndex] * (fileSize - i));
  }

  outReader = Reader(data, fileSize);
}

///
/// BANDITS - Phoenix Rising contains a .enc format used to store certain DieselScript and XML files ({original_extension}.enc)
/// 
/// The encryption method is quite simple:
/// It is a list of values to Xor the original data against (the encryption key), and a counter to keep track of where it is in the key.
/// After it reaches index 32 it rolls over back to zero and continues iterating on the buffer until it reaches the end.
/// 
/// The implementation can be found in BANDITS in the third entry of the dsl::EncryptedArchiveTokenizer virtual function table (Runtime Type Information is left enabled in BANDITS, so the only hurdle is SafeDisc)
/// 
/// To be able to view the implementation free of SafeDisc's encryption, you need to use a debugger like x32dbg on the executable (You will need to use ScyllaHide to get around SafeDisc's anti-debugger).
/// Place a breakpoint at the beginning of the address specified in the virtual function table.
/// Once this breakpoint has been hit, the function bytecode should be visible in your debugger.
/// You can now dump the executable using Scylla (or the equivalent in your debugger) to get an executable you can use in the reverse engineering software of your choice.
/// 
/// For those wishing to have BANDITS use your non-encrypted, edited files instead of the encrypted ones: BANDITS will load any file instead of the encrypted version if the filename is the same, but without ".enc" at the end.
///

char BANDITSXorEncryptionKey[] = { (char)0xDE, (char)0x5E, (char)0xDA, (char)0x07,
                  (char)0xBB, (char)0x96, (char)0x42, (char)0x40,
                  (char)0xB4, (char)0xEB, (char)0x64, (char)0xAD,
                  (char)0x6D, (char)0x8C, (char)0x24, (char)0x40,
                  (char)0x83, (char)0xEB, (char)0x3B, (char)0x3D,
                  (char)0xD6, (char)0x16, (char)0x1E, (char)0x97,
                  (char)0xB5, (char)0x70, (char)0x79, (char)0xD9,
                  (char)0x04, (char)0xED, (char)0x2B, (char)0x7E,
};

void Reader::ReadBANDITSEncryptedFile(Reader& outReader) {
  return ReadWithXORKey(outReader, BANDITSXorEncryptionKey, sizeof(BANDITSXorEncryptionKey));
}

bool Reader::Valid() const
{
  return this->container != nullptr;
}

std::string Reader::ReadString() {
  std::string str = "";

  char c = ReadType<char>();
  while (c != 0x00) {
    str += c;
    c = ReadType<char>();
  }

  return str;
}

std::string Reader::ReadLengthPrefixedString() {
  auto stringLength = ReadType<uint32_t>();

  // DANGER: Using this in the wrong place will allocate up to 4gb of memory at once.
  char* str = new char[stringLength];
  ReadBytesToBuffer(str, stringLength);

  std::string cpp_str = std::string(str, str + stringLength);

  delete[] str;

  return cpp_str;
}
