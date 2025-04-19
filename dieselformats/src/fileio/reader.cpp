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
    throw std::runtime_error("FileReaderContainer read beyond its size");
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
  if (this->data)
    delete[] this->data;
}

void MemoryReaderContainer::Close() {
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

void Reader::ReadCompressed(Reader& outReader) {
  auto uncompressedSize = this->ReadType<uint64_t>(); // could be uint32_t on 32bit diesel
  char* uncompressed = new char[uncompressedSize];

  auto compressedSize = this->ReadType<uint32_t>();

  char* compressed = new char[compressedSize];

  this->ReadBytesToBuffer(compressed, compressedSize);

  compression::ZlibDecompression::DecompressBuffer(compressed, compressedSize, uncompressed, uncompressedSize);

  delete[] compressed;

  outReader = Reader(uncompressed, uncompressedSize);
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

// Really an array of chars, making it an array of ints removes compiler warnings
int BANDITSXorEncryptionKey[] = { 0xDE, 0x5E, 0xDA, 0x07,
                  0xBB, 0x96, 0x42, 0x40,
                  0xB4, 0xEB, 0x64, 0xAD,
                  0x6D, 0x8C, 0x24, 0x40,
                  0x83, 0xEB, 0x3B, 0x3D,
                  0xD6, 0x16, 0x1E, 0x97,
                  0xB5, 0x70, 0x79, 0xD9,
                  0x04, 0xED, 0x2B, 0x7E,
};

void Reader::ReadBANDITSEncryptedFile(Reader& outReader) {
  auto fileSize = this->GetFileSize();
  char* data = new char[fileSize];
  this->ReadBytesToBuffer(data, fileSize);

  int keyIndex = 0;

  for (int i = 0; i < fileSize; i++) {
    data[i] = data[i] ^ (char)BANDITSXorEncryptionKey[keyIndex];

    keyIndex++;
    if (keyIndex >= 32) // could probably be: "data[i] ^ BANDITSXorEncryptionKey[keyIndex % sizeof(encryptionKey)]" instead of this
      keyIndex = 0;
  }

  outReader = Reader(data, fileSize);
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
