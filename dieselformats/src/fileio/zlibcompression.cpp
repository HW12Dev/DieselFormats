#include "zlibcompression.h"

#include <zlib.h>

void compression::ZlibDecompression::DecompressBuffer(char* inputBuffer, std::size_t inputBufferSize, char* outputBuffer, std::size_t outputBufferSize) {
  if (inputBuffer == nullptr || inputBufferSize == 0)
    return;


  z_stream inflateStream{};

  inflateStream.zalloc = Z_NULL;
  inflateStream.zfree = Z_NULL;
  inflateStream.opaque = Z_NULL;
  inflateStream.avail_in = 0;
  inflateStream.next_in = Z_NULL;

  if (inflateInit2(&inflateStream, (15 + 32)) != Z_OK)
    return;

  inflateStream.next_in = (Bytef*)inputBuffer;
  inflateStream.avail_in = inputBufferSize;

    inflateStream.avail_out = outputBufferSize;
    inflateStream.next_out = (Bytef*)outputBuffer;

  auto inflateRet = inflate(&inflateStream, Z_NO_FLUSH);
  if (inflateRet != Z_OK && inflateRet != Z_STREAM_END) {
    __debugbreak();
  }

  inflateEnd(&inflateStream);
}
