#include "writer.h"

#include "reader.h"

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
