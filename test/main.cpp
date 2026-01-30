#include <iostream>
#include <string>
#include <vector>
#include "parsing/TorrentFile.h"
#include "tracker/Tracker.h"
#include "download/Downloader.h"

int main(int argc, char* argv[]) {
    // 1. Check Args
    if (argc < 2) {
        std::cerr << "Usage: ./my_torrent_client <file.torrent>\n";
        return 1;
    }

    try {
        // 2. Load Torrent File
        std::cout << "Loading torrent file...\n";
        auto torrent = BitTorrent::TorrentFile::Load(argv[1]);
        
        // 3. Generate Peer ID
        std::string my_id = "-CPP-CLIENT-0001-XYZ";

        // 4. Get Peers from Tracker
        std::cout << "Connecting to Tracker...\n";
        auto peers = BitTorrent::Tracker::GetPeers(torrent, my_id);
        
        if (peers.empty()) {
            std::cerr << "No peers found! Aborting.\n";
            return 1;
        }
        std::cout << "Found " << peers.size() << " peers.\n";

        // 5. Start Download
        BitTorrent::Downloader d(torrent, my_id, peers);
        d.Start();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}