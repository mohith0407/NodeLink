#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "parsing/Buffer.h"        
#include "parsing/TorrentFile.h"   

namespace BitTorrent {

    class Message {
    public:
        // Standard BitTorrent Message IDs
        static constexpr uint8_t CHOKE = 0;
        static constexpr uint8_t UNCHOKE = 1;
        static constexpr uint8_t INTERESTED = 2;
        static constexpr uint8_t NOT_INTERESTED = 3;
        static constexpr uint8_t HAVE = 4;
        static constexpr uint8_t BITFIELD = 5;
        static constexpr uint8_t REQUEST = 6;
        static constexpr uint8_t PIECE = 7;
        static constexpr uint8_t CANCEL = 8;

        // --- Builders (Outgoing) ---
        static Buffer BuildHandshake(const TorrentFile& t, const std::string& peer_id);
        static Buffer BuildKeepAlive();
        static Buffer BuildChoke();
        static Buffer BuildUnchoke();
        static Buffer BuildInterested();
        static Buffer BuildRequest(uint32_t index, uint32_t begin, uint32_t length);
        
        // --- Parsers (Incoming) ---
        // Reads the 4-byte length prefix from a buffer
        static uint32_t ReadMessageLength(const Buffer& b);
        // Reads the Message ID (e.g., 7 for PIECE)
        static uint8_t ReadMessageID(const Buffer& b);
    };
}