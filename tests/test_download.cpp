#include "download/downloader.h"
#include "tracker/tracker.h"
#include <iostream>
#include <random>

std::string generatePeerID() {
    return "-GG1000-GOOGLECLIENT"; // Fixed 20 chars
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./bittorrent_client <file.torrent>" << std::endl;
        return 1;
    }

    try {
        std::cout << "=== BitTorrent Client Starting ===" << std::endl;
        
        // 1. Parse
        BitTorrent::Torrent t(argv[1]);
        std::cout << "File: " << t.name << " (" << t.length << " bytes)" << std::endl;

        // 2. Tracker
        std::string my_id = generatePeerID();
        std::cout << "Getting peers..." << std::endl;
        auto peers = BitTorrent::Tracker::GetPeers(t, my_id, 6881);
        
        if (peers.empty()) {
            std::cerr << "No peers found. Aborting." << std::endl;
            return 1;
        }

        // 3. Download
        BitTorrent::Downloader d(t, my_id);
        d.Start(peers);

        std::cout << "Download Finished!" << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}