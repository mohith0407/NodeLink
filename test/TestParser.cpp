#include "parsing/TorrentFile.h"
#include "parsing/Buffer.h"
#include <iostream>
#include <cassert>

using namespace BitTorrent;

void TestBuffer() {
    std::cout << "[Test] Buffer Utils..." << std::endl;
    Buffer b = {0x00, 0x00, 0x01, 0x02}; // 258
    assert(BufferUtils::ReadBE32(b, 0) == 258);
    std::cout << "PASS" << std::endl;
}

void TestBencode() {
    std::cout << "[Test] Bencode Parsing..." << std::endl;
    std::string raw = "d3:key5:valuee"; // {"key": "value"}
    Buffer b(raw.begin(), raw.end());
    
    Bnode root = Bnode::Decode(b);
    assert(root.IsDict());
    assert(root.GetDict().at("key").GetString() == "value");
    std::cout << "PASS" << std::endl;
}
// argc (Argument Count): How many words did you type in the terminal?
// argv (Argument Vector): An array of the actual words you typed.
int main(int argc, char* argv[]) {
    try {
        TestBuffer();
        TestBencode();

        if (argc > 1) {
            std::cout << "[Test] Loading Torrent: " << argv[1] << std::endl;
            TorrentFile t = TorrentFile::Load(argv[1]);
            std::cout << "Name: " << t.name << std::endl;
            std::cout << "Announce: " << t.announce << std::endl;
            std::cout << "Size: " << t.length << " bytes" << std::endl;
            std::cout << "Pieces: " << t.piece_hashes.size() << std::endl;
            std::cout << "Info Hash: ";
            for(auto c : t.info_hash) printf("%02x", c);
            std::cout << std::endl;
        } else {
            std::cout << "Note: Pass a .torrent file to test full loading." << std::endl;
        }

    } catch (std::exception& e) {
        std::cerr << "FAIL: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}