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
  inflateInit(&inflateStream);

  inflateStream.next_in = (Bytef*)inputBuffer;
  inflateStream.avail_in = inputBufferSize;

  inflateStream.avail_out = outputBufferSize;
  inflateStream.next_out = (Bytef*)outputBuffer;

  auto inflateRet = inflate(&inflateStream, Z_NO_FLUSH);

  inflateEnd(&inflateStream);

  if (inflateRet != Z_OK && inflateRet != Z_STREAM_END) {
    __debugbreak();
  }

}

// https://github.com/madler/zlib/blob/5a82f71ed1dfc0bec044d9702463dbdf84ea3b71/compress.c#L14
std::size_t compression::ZlibDecompression::GetRecommendedCompressionBufferSize(std::size_t inputSize) {
  return inputSize + (inputSize * 0.001f) + 12;
}

std::size_t compression::ZlibDecompression::CompressBuffer(char* inputBuffer, std::size_t inputBufferSize, char* outputBuffer, std::size_t outputBufferSize, int compressionLevel) {
  if (inputBuffer == nullptr || inputBufferSize == 0)
    return -1;

  uLongf out = outputBufferSize;
  int compressRet = compress2((Bytef*)outputBuffer, &out, (Bytef*)inputBuffer, inputBufferSize, compressionLevel);

  if (compressRet == Z_BUF_ERROR)
    return -1;

  return out;
}
