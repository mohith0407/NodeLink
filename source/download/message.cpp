#include "download/Message.h"
#include <cstring>
#include <iterator>
// just a traslator to network form local and vice versa
namespace BitTorrent {

    // Helper: Appends a 32-bit Big Endian integer to the buffer
    // We implement this locally to avoid dependency issues with external Utils for now.
    void PushInt32(Buffer& b, uint32_t val) {
        b.push_back((val >> 24) & 0xFF);
        b.push_back((val >> 16) & 0xFF);
        b.push_back((val >> 8) & 0xFF);
        b.push_back(val & 0xFF);
    }

    Buffer Message::BuildHandshake(const TorrentFile& t, const std::string& peer_id) {
        Buffer hs;
        // 1. PSTR Len (19)
        hs.push_back(19); 
        
        // 2. PSTR ("BitTorrent protocol")
        std::string proto = "BitTorrent protocol"; // proto.size=19 
        hs.insert(hs.end(), proto.begin(), proto.end());
        
        // 3. Reserved (8 bytes of zeros)
        for(int i=0; i<8; i++) hs.push_back(0); 
        
        // 4. Info Hash (20 bytes) - From the parsing module
        // "I want the file with THIS ID".
        hs.insert(hs.end(), t.info_hash.begin(), t.info_hash.end());
        
        // 5. Peer ID (20 bytes)
        // "My name is -CPP-CLIENT..."
        hs.insert(hs.end(), peer_id.begin(), peer_id.end());
        
        return hs;
    }

    Buffer Message::BuildKeepAlive() { 
        return {0, 0, 0, 0}; 
    }
    // These are "Length-Prefixed" messages. 
    // Format: [Length (4 bytes)] [ID (1 byte)]
    Buffer Message::BuildChoke() { 
        Buffer b; 
        PushInt32(b, 1);     // Length: 1
        b.push_back(CHOKE);  // ID: 0
        return b; 
    }

    Buffer Message::BuildUnchoke() { 
        Buffer b; 
        PushInt32(b, 1); 
        b.push_back(UNCHOKE); 
        return b; 
    }

    Buffer Message::BuildInterested() { 
        Buffer b; 
        PushInt32(b, 1); 
        b.push_back(INTERESTED); 
        return b; 
    }

    Buffer Message::BuildRequest(uint32_t index, uint32_t begin, uint32_t length) {
        Buffer b;
        PushInt32(b, 13); // Length = 13 (1 byte ID + 12 bytes payload(4 Index + 4 Begin + 4 Len))
        b.push_back(REQUEST);
        PushInt32(b, index);
        PushInt32(b, begin);
        PushInt32(b, length);
        return b;
    }

    uint32_t Message::ReadMessageLength(const Buffer& b) {
        if(b.size() < 4) return 0;
        // Reconstruct the integer from 4 bytes (Big Endian -> Host Endian)
        return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    }

    uint8_t Message::ReadMessageID(const Buffer& b) {
        if(b.size() < 5) return 0; // Should check length first
        return b[4];
    }
    // Scenario:
    // We read 4 bytes: 00 00 40 09 (Hex for 16393).
    // ReadMessageLength returns 16393.
    // Connection.cpp now knows: "Okay, I need to wait until I have 16393 more bytes in the buffer before I can process this message."
}

// Message Name,ID,Length,Purpose
// Handshake,-,68,"""Hello, let's talk about file X."""
// KeepAlive,-,0,"""Don't disconnect me."""
// Choke,0,1,"""Stop asking me for data."""
// Unchoke,1,1,"""Okay, you can ask for data now."""
// Interested,2,1,"""I want what you have."""
// Have,4,5,"""I just finished downloading Piece #X."""
// Request,6,13,"""Send me data!"""
// Piece,7,9 + Data,"""Here is the data you asked for."""