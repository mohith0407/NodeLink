#pragma once
#include <string>
#include <cstdint>

namespace BitTorrent {

    struct Peer {
        std::string ip;
        uint16_t port;
        std::string id; // The peer's unique ID (optional, but good for debugging)

        // Constructor
        Peer(std::string i, uint16_t p, std::string pid = "") 
            : ip(i), port(p), id(pid) {}
            
        // Default Constructor
        Peer() : port(0) {}
    };
}