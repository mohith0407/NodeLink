#ifndef TRACKER_H
#define TRACKER_H

#include "parsing/torrent.h"
#include "tracker/tracker_utils.h"
#include <vector>
#include <string>

namespace BitTorrent {

    struct Peer {
        std::string ip;
        int port;
    };

    class Tracker {
    public:
        // Main function: Give me a Torrent, I give you a list of Peers
        static std::vector<Peer> GetPeers(const Torrent& t, const std::string& peer_id, int listening_port);

    private:
        static std::vector<Peer> GetPeersUDP(const Url& url, const Torrent& t, const std::string& peer_id, int port);
        static std::vector<Peer> GetPeersHTTP(const Url& url, const Torrent& t, const std::string& peer_id, int port);
    };
}
#endif