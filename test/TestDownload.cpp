#include "parsing/TorrentFile.h"
#include "tracker/Tracker.h"
#include "download/Downloader.h"
#include <iostream>
#include <random>

using namespace BitTorrent;

// Helper to generate a random Peer ID
std::string GenerateID() {
    std::string id = "-CPP100-";
    std::srand(time(0));
    for (int i = 0; i < 12; ++i) id += std::to_string(std::rand() % 10);
    return id;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./test_download <file.torrent>" << std::endl;
        return 1;
    }

    try {
        // 1. Load Metadata
        std::cout << "Loading " << argv[1] << "..." << std::endl;
        TorrentFile tf = TorrentFile::Load(argv[1]);
        std::string my_id = GenerateID();

        // 2. Get Peers
        std::cout << "Contacting Tracker..." << std::endl;
        std::vector<Peer> peers = Tracker::GetPeers(tf, my_id);
        
        if (peers.empty()) {
            std::cerr << "No peers found! Try a popular torrent." << std::endl;
            return 1;
        }
        std::cout << "Found " << peers.size() << " peers." << std::endl;

        // 3. Start Download
        Downloader d(tf, my_id, peers);
        d.Start();

    } catch (std::exception& e) {
        std::cerr << "CRASH: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}