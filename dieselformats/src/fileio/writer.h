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


  void WriteBytes(char* inBuffer, std::size_t size);

  // write the remaining contents of a reader object
  void WriteReader(Reader& reader);

  template<typename T> void WriteType(const T& value) {
    this->WriteBytes((char*)&value, sizeof(T));
  }


private:
  unsigned long long position;
  HANDLE file;
};