#include "tracker/Tracker.h"
#include <iostream>
#include <random>

using namespace BitTorrent;

std::string GeneratePeerID() {
    std::string id = "-BT1000-"; // Client Prefix
    std::srand(time(0));
    for (int i = 0; i < 12; ++i) id += std::to_string(std::rand() % 10);
    return id;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./test_tracker <file.torrent>" << std::endl;
        return 1;
    }

    try {
        // 1. Load File
        std::cout << "Loading " << argv[1] << "..." << std::endl;
        TorrentFile tf = TorrentFile::Load(argv[1]);

        // 2. Generate ID
        std::string my_id = GeneratePeerID();

        // 3. Ask Tracker
        std::vector<Peer> peers = Tracker::GetPeers(tf, my_id);

        // 4. Print Results
        std::cout << "Found " << peers.size() << " peers:" << std::endl;
        for (const auto& p : peers) {
            std::cout << " - " << p.ip << ":" << p.port << std::endl;
        }

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}