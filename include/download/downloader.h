#pragma once
#include "parsing/TorrentFile.h"
#include "tracker/Peer.h"
#include "download/Worker.h"
#include <vector>
#include <atomic>
#include <string>

namespace BitTorrent {

    class Downloader {
    public:
        TorrentFile torrent;
        std::string my_id;
        std::vector<Peer> peers;
        
        // These are from Step 2
        Writer file_writer;
        Speed s;
        void Start();
        std::atomic<int> next_req_index{0};
        std::atomic<long long> downloaded_bytes{0};

        // Constructor
        Downloader(const TorrentFile& tf, const std::string& id, const std::vector<Peer>& p_list);

        // Virtual methods allow us to Mock them in tests
        virtual void OnBlockReceived(int piece_index, int offset, Buffer& data);
        
        virtual int GetNextPieceToRequest() { 
            int idx = next_req_index++;
            if(idx >= torrent.piece_hashes.size()) return -1;
            return idx;
        }

        bool IsComplete() {
            return downloaded_bytes >= torrent.length;
        }
    };
}