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

  if (size + position > this->GetFileSize())
    return 0;

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
  memcpy(outBuffer, &this->data[position], size);

  return size;
}

#pragma endregion

Reader::Reader() {
  this->isFileBased = false;
  this->container = nullptr;
  this->position = -1;
  this->replacementSize = -1;
}

Reader::Reader(const std::filesystem::path& path) {
  this->isFileBased = true;
  this->container = std::make_shared<FileReaderContainer>(path);
  this->position = 0;
  this->replacementSize = -1;
}

Reader::Reader(char* buffer, std::size_t size) {
  this->isFileBased = false;
  this->container = std::make_shared<MemoryReaderContainer>(buffer, size);
  this->position = 0;
  this->replacementSize = -1;
}

Reader::~Reader() {
}

Reader::Reader(const Reader& other)
{
  *this = other;
}

Reader& Reader::operator=(const Reader& other)
{
  this->isFileBased = other.isFileBased;
  this->container = other.container;
  this->position = other.position;
  this->replacementSize = other.replacementSize;

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

void Reader::Close() {
  if (this->container)
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

bool Reader::Valid() const
{
  return this->container != nullptr;
}

std::string Reader::ReadString() {
  std::string str = "";

  char c = this->ReadType<char>();
  while (c != 0x00) {
    str += c;
    c = this->ReadType<char>();
  }

  return str;
}