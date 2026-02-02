#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

class Sha1 {
public:
    static std::string Calculate(const std::vector<uint8_t>& data) {
        uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;
        uint64_t ml = data.size() * 8;
        std::vector<uint8_t> m = data;

        m.push_back(0x80);
        while ((m.size() * 8) % 512 != 448) m.push_back(0);
        for (int i = 0; i < 8; i++) m.push_back((ml >> (56 - i * 8)) & 0xFF);

        for (size_t i = 0; i < m.size(); i += 64) {
            uint32_t w[80], a = h0, b = h1, c = h2, d = h3, e = h4;
            for (int j = 0; j < 16; j++) 
                w[j] = (m[i + j * 4] << 24) | (m[i + j * 4 + 1] << 16) | (m[i + j * 4 + 2] << 8) | m[i + j * 4 + 3];
            for (int j = 16; j < 80; j++) 
                w[j] = LeftRotate((w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16]), 1);

            for (int j = 0; j < 80; j++) {
                uint32_t f, k;
                if (j < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
                else if (j < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
                else if (j < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
                else { f = b ^ c ^ d; k = 0xCA62C1D6; }
                uint32_t temp = LeftRotate(a, 5) + f + e + k + w[j];
                e = d; d = c; c = LeftRotate(b, 30); b = a; a = temp;
            }
            h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
        }

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(8) << h0 << std::setw(8) << h1 << std::setw(8) << h2 << std::setw(8) << h3 << std::setw(8) << h4;
        return ss.str();
    }

private:
    static uint32_t LeftRotate(uint32_t x, uint32_t c) { return (x << c) | (x >> (32 - c)); }
};