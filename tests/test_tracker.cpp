#include "parsing/torrent.h"
#include "tracker/tracker.h"
#include <iostream>
#include <random>

std::string generatePeerID() {
    std::string id = "-GG1000-"; 
    while(id.size() < 20) id += std::to_string(rand() % 10);
    return id;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./test_tracker <file.torrent>" << std::endl;
        return 1;
    }

    try {
        // 1. Parse
        BitTorrent::Torrent t(argv[1]);
        std::cout << "Torrent: " << t.name << std::endl;
        std::cout << "Tracker URL: " << t.tracker_url << std::endl;

        // 2. Contact Tracker
        std::string myID = generatePeerID();
        std::cout << "My Peer ID: " << myID << std::endl;

        std::vector<BitTorrent::Peer> peers = BitTorrent::Tracker::GetPeers(t, myID, 6881);

        // 3. Results
        std::cout << "\n[SUCCESS] Found " << peers.size() << " peers!" << std::endl;
        for (size_t i = 0; i < std::min((size_t)10, peers.size()); ++i) {
            std::cout << "  - " << peers[i].ip << ":" << peers[i].port << std::endl;
        }

    } catch (std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}