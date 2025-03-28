#pragma once

#include <cstddef>

namespace compression {
  class ZlibDecompression {
  private:
    ZlibDecompression() = delete;

  public:
    static void DecompressBuffer(char* inputBuffer, std::size_t inputBufferSize, char* outputBuffer, std::size_t outputBufferSize);
  };
}