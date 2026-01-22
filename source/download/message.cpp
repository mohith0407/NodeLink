#include "download/message.h"
#include <cstring>
#include <iterator>

namespace BitTorrent {

    Buffer Message::BuildHandshake(const Torrent& t, const std::string& peer_id) {
        Buffer packet;
        // 1. PSTR Length (19)
        packet.push_back(19);
        // 2. PSTR ("BitTorrent protocol")
        std::string pstr = "BitTorrent protocol";
        packet.insert(packet.end(), pstr.begin(), pstr.end());
        // 3. Reserved (8 zero bytes)
        packet.insert(packet.end(), 8, 0);
        // 4. Info Hash (20 bytes)
        packet.insert(packet.end(), t.info_hash.begin(), t.info_hash.end());
        // 5. Peer ID (20 bytes)
        packet.insert(packet.end(), peer_id.begin(), peer_id.end());
        
        return packet;
    }

    Buffer Message::BuildRequest(uint32_t index, uint32_t begin, uint32_t length) {
        // Format: <Length=13><ID=6><Index><Begin><Length>
        Buffer packet(17);
        BufferUtils::WriteBE32(packet, 0, 13); // Length
        packet[4] = REQUEST;                   // ID
        BufferUtils::WriteBE32(packet, 5, index);
        BufferUtils::WriteBE32(packet, 9, begin);
        BufferUtils::WriteBE32(packet, 13, length);
        return packet;
    }

    Buffer Message::BuildInterested() {
        // Format: <Length=1><ID=2>
        Buffer packet(5);
        BufferUtils::WriteBE32(packet, 0, 1);
        packet[4] = INTERESTED;
        return packet;
    }
    
    Buffer Message::BuildKeepAlive() {
        Buffer packet(4);
        BufferUtils::WriteBE32(packet, 0, 0);
        return packet;
    }

    uint32_t Message::ReadMessageLength(const Buffer& b) {
        if (b.size() < 4) return 0;
        return BufferUtils::ReadBE32(b, 0);
    }
    
    uint8_t Message::ReadMessageID(const Buffer& b) {
        if (b.size() < 5) return 0; // Keep Alive has no ID
        return b[4];
    }
}