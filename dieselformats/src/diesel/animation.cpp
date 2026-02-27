#include "diesel/animation.h"

#include "fileio/zlibcompression.h"


#include "fileio/writer.h"

using namespace diesel;

bool Animation::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  size_t compressedBufferSize = reader.GetFileSize() - 4;

  size_t start = reader.GetPosition();

  reader.AddPosition(compressedBufferSize);

  size_t uncompressedBufferSize = reader.ReadType<uint32_t>();

  reader.SetPosition(start);

  char* uncompressedBuffer = new char[uncompressedBufferSize] {};
  char* compressedBuffer = new char[compressedBufferSize] {};

  reader.ReadBytesToBuffer(compressedBuffer, compressedBufferSize);

  compression::ZlibDecompression::DecompressBuffer(compressedBuffer, compressedBufferSize, uncompressedBuffer, uncompressedBufferSize);

  delete[] compressedBuffer;

  Reader uncompressedReader(uncompressedBuffer, uncompressedBufferSize);


  return ReadUncompressed(uncompressedReader, version);
}



bool Animation::ReadUncompressed(Reader& reader, const DieselFormatsLoadingParameters& version) {
  size_t start = reader.GetPosition();

  
  if (diesel::AreLoadParameters32Bit(version)) {
    reader.AddPosition(16);
  }
  else {
    reader.AddPosition(32);
  }

  _length = reader.ReadType<float>();

  if (!diesel::AreLoadParameters32Bit(version)) {
    reader.AddPosition(4); // class alignment
  }

  InplaceArray<InplaceString> names(reader, version);
  

  reader.SetPosition(start + names._data);

  for (int i = 0; i < names._n; i++) {
    InplaceString str(reader, version);

    size_t returnPos = reader.GetPosition();

    reader.SetPosition(start + str._s);

    _names.push_back(reader.ReadString());

    reader.SetPosition(returnPos);
    
  }

  return true;
}


