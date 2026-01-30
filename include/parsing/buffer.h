#pragma once
#include <vector>
#include <string>
#include <cstdint> // for uint8_t, uint32_t
#include <stdexcept>

namespace BitTorrent {

    // A Buffer is just a list of bytes.
    using Buffer = std::vector<uint8_t>;

    class BufferUtils {
    public:
        // Reads a 32-bit Integer (4 bytes) from a buffer in Big Endian
        static uint32_t ReadBE32(const Buffer& buffer, size_t index) {
            if (index + 4 > buffer.size()) throw std::out_of_range("Buffer read overflow");
            return (buffer[index] << 24) | (buffer[index + 1] << 16) | 
                   (buffer[index + 2] << 8) | buffer[index + 3];
        }

        // Reads a 16-bit Integer (2 bytes)
        static uint16_t ReadBE16(const Buffer& buffer, size_t index) {
            if (index + 2 > buffer.size()) throw std::out_of_range("Buffer read overflow");
            return (buffer[index] << 8) | buffer[index + 1];
        }

        // Writes a 32-bit Integer
        static void WriteBE32(Buffer& buffer, uint32_t value) {
            buffer.push_back((value >> 24) & 0xFF);
            buffer.push_back((value >> 16) & 0xFF);
            buffer.push_back((value >> 8) & 0xFF);
            buffer.push_back(value & 0xFF);
        }

        // Helper to convert std::string to Buffer
        static Buffer FromString(const std::string& str) {
            return Buffer(str.begin(), str.end());
        }
        
        // Helper to convert Buffer to std::string (for printing)
        static std::string ToString(const Buffer& buffer) {
            return std::string(buffer.begin(), buffer.end());
        }
    };
}