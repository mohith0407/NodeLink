#pragma once
#include "parsing/TorrentFile.h"
#include "tracker/Url.h"
#include "tracker/Peer.h"
#include <vector>

namespace BitTorrent {

    class Tracker {
    public:
        // Main Function: Input Torrent -> Output Peers
        static std::vector<Peer> GetPeers(const TorrentFile& tf, const std::string& peer_id, int port = 6881);

    private:
        static std::vector<Peer> GetPeersHTTP(const Url& url, const TorrentFile& tf, const std::string& peer_id, int port);
        static std::vector<Peer> GetPeersUDP(const Url& url, const TorrentFile& tf, const std::string& peer_id, int port);
        
        static std::string UrlEncode(const Buffer& buffer);
    };
}