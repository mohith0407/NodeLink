#include "parsing/buffer.h"

namespace BitTorrent {

    uint16_t BufferUtils::ReadBE16(const Buffer& b, size_t idx) {
        if (idx + 1 >= b.size()) throw std::out_of_range("Buffer index out of bounds");
        return (b[idx] << 8) | b[idx + 1];
    }

    uint32_t BufferUtils::ReadBE32(const Buffer& b, size_t idx) {
        if (idx + 3 >= b.size()) throw std::out_of_range("Buffer index out of bounds");
        uint32_t ans = 0;
        for (int i = 0; i < 4; i++) {
            ans = (ans << 8) | b[idx + i];
        }
        return ans;
    }

    uint64_t BufferUtils::ReadBE64(const Buffer& b, size_t idx) {
        if (idx + 7 >= b.size()) throw std::out_of_range("Buffer index out of bounds");
        uint64_t ans = 0;
        for (int i = 0; i < 8; i++) {
            ans = (ans << 8) | b[idx + i];
        }
        return ans;
    }

    void BufferUtils::WriteBE16(Buffer& b, size_t idx, uint16_t n) {
        if (idx + 1 >= b.size()) throw std::out_of_range("Buffer index out of bounds");
        b[idx + 1] = n & 0xff;
        b[idx] = (n >> 8) & 0xff;
    }

    void BufferUtils::WriteBE32(Buffer& b, size_t idx, uint32_t n) {
        if (idx + 3 >= b.size()) throw std::out_of_range("Buffer index out of bounds");
        for (int i = 0; i < 4; i++) {
            b[idx + 3 - i] = n & 0xff;
            n >>= 8;
        }
    }

    void BufferUtils::WriteBE64(Buffer& b, size_t idx, uint64_t n) {
        if (idx + 7 >= b.size()) throw std::out_of_range("Buffer index out of bounds");
        for (int i = 0; i < 8; i++) {
            b[idx + 7 - i] = n & 0xff;
            n >>= 8;
        }
    }

    void BufferUtils::PrintHex(const Buffer& b) {
        for (auto byte : b) {
            std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                      << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << std::endl;
    }
}