#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace BitTorrent {

    // "Buffer" is just a list of bytes (unsigned chars).
    // We use a typedef to make the code easier to read.
    using Buffer = std::vector<uint8_t>;

    class BufferUtils {
    public:
        // Reads a 16-bit integer (2 bytes) from Big Endian format
        static uint16_t ReadBE16(const Buffer& b, size_t idx);
        
        // Reads a 32-bit integer (4 bytes)
        static uint32_t ReadBE32(const Buffer& b, size_t idx);
        
        // Reads a 64-bit integer (8 bytes) - Essential for large file sizes
        static uint64_t ReadBE64(const Buffer& b, size_t idx);
        
        // Writes integers back into the buffer in Big Endian
        static void WriteBE16(Buffer& b, size_t idx, uint16_t n);
        static void WriteBE32(Buffer& b, size_t idx, uint32_t n);
        static void WriteBE64(Buffer& b, size_t idx, uint64_t n);
        
        // Debug helper to see raw hex values
        static void PrintHex(const Buffer& b);
    };
}
#endif