#pragma once

#include <cstddef>

// 
// Diesel uses zlib 1.2.3 (released 2005)
// 

// zlib compression levels, from https://github.com/madler/zlib/blob/5a82f71ed1dfc0bec044d9702463dbdf84ea3b71/zlib.h#L194
#define ZLIB_COMPRESSION_LEVEL_NO_COMPRESSION         0
#define ZLIB_COMPRESSION_LEVEL_BEST_SPEED             1
#define ZLIB_COMPRESSION_LEVEL_BEST_COMPRESSION       9
#define ZLIB_COMPRESSION_LEVEL_DEFAULT_COMPRESSION  (-1)

static const unsigned long long Diesel_CompressedDataStore_ChunkSize = 0x10000; // ChunkSize / BlockSize

namespace compression {
  class ZlibDecompression {
  private:
    ZlibDecompression() = delete;

  public:
    static void DecompressBuffer(char* inputBuffer, std::size_t inputBufferSize, char* outputBuffer, std::size_t outputBufferSize);

    /// Returns the inputSize + 0.1% of the inputSize + 12
    static std::size_t GetRecommendedCompressionBufferSize(std::size_t inputSize);

    // Compress the provided input buffer into the provided output buffer.
    // The output buffer must be (as per zlib) at least 0.1% larger than the inputBuffer + 12 bytes. This value can be retrieved from GetMaximumCompressedBufferSize
    // On success: returns the size of the compressed data. On failure: returns -1.
    static std::size_t CompressBuffer(char* inputBuffer, std::size_t inputBufferSize, char* outputBuffer, std::size_t outputBufferSize, int compressionLevel);
  };
}