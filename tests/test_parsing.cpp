#include "parsing/torrent.h"
#include "parsing/buffer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./run_tests <file.torrent>" << std::endl;
        return 1;
    }

    try {
        std::cout << "Testing Parsing Layer..." << std::endl;
        
        // Test 1: Buffer Utils
        BitTorrent::Buffer b = {0x00, 0x01, 0x02, 0x03};
        uint32_t val = BitTorrent::BufferUtils::ReadBE32(b, 0);
        if (val != 66051) throw std::runtime_error("BufferUtils Failed");
        std::cout << "[PASS] BufferUtils" << std::endl;

        // Test 2: Torrent Parsing
        BitTorrent::Torrent t(argv[1]);
        std::cout << "[PASS] Torrent Parsed Successfully!" << std::endl;
        std::cout << "   Name: " << t.name << std::endl;
        std::cout << "   Size: " << t.length << " bytes" << std::endl;
        std::cout << "   Tracker: " << t.tracker_url << std::endl;

    } catch (std::exception& e) {
        std::cerr << "[FAIL] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}