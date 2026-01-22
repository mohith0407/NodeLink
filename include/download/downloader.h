#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "tracker/tracker.h"
#include "parsing/torrent.h"
#include "download/file_manager.h"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

namespace BitTorrent {

    class Downloader {
    public:
        Downloader(const Torrent& t, const std::string& my_peer_id);
        void Start(const std::vector<Peer>& peers);

    private:
        Torrent torrent;
        std::string my_id;
        FileManager file_manager;
        
        // State
        std::vector<bool> pieces_complete;
        std::mutex piece_mutex;
        std::atomic<int> pieces_downloaded_count;

        // Worker Function (Runs in a thread)
        void PeerWorker(Peer p);

        // Helper to find the next missing piece safely
        int GetNextPieceToDownload(const std::vector<bool>& peer_bitfield);
        
        // Helper to verify hash
        bool VerifyPiece(const Buffer& piece_data, int piece_index);
    };
}
#endif