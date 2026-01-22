#ifndef MESSAGE_H
#define MESSAGE_H

#include "parsing/buffer.h"
#include "parsing/torrent.h"
#include <string>

namespace BitTorrent {

    class Message {
    public:
        enum ID {
            CHOKE = 0,
            UNCHOKE = 1,
            INTERESTED = 2,
            NOT_INTERESTED = 3,
            HAVE = 4,
            BITFIELD = 5,
            REQUEST = 6,
            PIECE = 7,
            CANCEL = 8
        };

        // Builds the initial handshake packet (Length 68 bytes)
        static Buffer BuildHandshake(const Torrent& t, const std::string& peer_id);
        
        // Builds a request for a specific block of data
        static Buffer BuildRequest(uint32_t index, uint32_t begin, uint32_t length);
        
        // Simple state messages
        static Buffer BuildInterested();
        static Buffer BuildKeepAlive();

        // Helper to read the message length prefix (4 bytes)
        static uint32_t ReadMessageLength(const Buffer& b);
        
        // Helper to parse the ID from a message
        static uint8_t ReadMessageID(const Buffer& b);
    };
}
#endif