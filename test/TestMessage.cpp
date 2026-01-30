#include <iostream>
#include <iomanip>
#include "download/Message.h"
#include "parsing/TorrentFile.h"
#include "parsing/Bnode.h" // Depending on how you include the parser

// Usage: ./step1_test <path_to_torrent_file>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.torrent>\n";
        return 1;
    }

    try {
        // 1. Integration: Use your REAL parsing logic
        std::cout << "[Test] Parsing torrent file: " << argv[1] << "...\n";
        BitTorrent::TorrentFile t = BitTorrent::TorrentFile::Load(argv[1]);
        
        std::cout << "[Test] Torrent Name: " << t.name << "\n";
        std::cout << "[Test] Info Hash Size: " << t.info_hash.size() << " bytes\n";

        // 2. Integration: Build Handshake using the parsed data
        std::string my_id = "-CPP01-1234567890123"; // 20 chars
        std::cout << "[Test] Building Handshake...\n";
        
        BitTorrent::Buffer hs = BitTorrent::Message::BuildHandshake(t, my_id);

        // 3. Validation
        std::cout << "[Test] Handshake Total Size: " << hs.size() << " (Expected 68)\n";
        
        if (hs.size() != 68) {
            std::cerr << "[FAIL] Handshake size is incorrect!\n";
            return 1;
        }

        std::cout << "[PASS] Handshake constructed successfully.\n";
        std::cout << "[PASS] Protocol: " << std::string(hs.begin()+1, hs.begin()+20) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }

    return 0;
}