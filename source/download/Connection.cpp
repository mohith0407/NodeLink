#include "download/Connection.h"
#include "download/Downloader.h"
#include "parsing/buffer.h"
#include "download/Message.h"
#include <iostream>

namespace BitTorrent {

    // Local Helper
    // uint32_t ReadInt32(const Buffer& b, size_t offset) {
    //     if (b.size() < offset + 4) return 0;
    //     return (b[offset] << 24) | (b[offset+1] << 16) | (b[offset+2] << 8) | b[offset+3];
    // }

    Connection::Connection(Peer p, Downloader& d) : peer(p), downloader(d) {}

    void Connection::Connect() {
        try {
            socket = std::make_unique<TcpClient>(peer.ip, peer.port);
            socket->SetTimeout(2); 
            state = CONNECTING;
        } catch(...) {
            socket = nullptr;
        }
    }

    void Connection::OnReadyWrite() {
        if (!socket) return;

        if (state == CONNECTING) {
            try {
                Buffer hs = Message::BuildHandshake(downloader.torrent, downloader.my_id);
                socket->Send(hs);
                state = HANDSHAKING;
                std::cout << "[Conn] Connected & Handshake sent to " << peer.ip << std::endl;
            } catch (...) {
                socket = nullptr;
            }
        }
    }
    // The Concept: TCP is a Stream, not a message queue. If Peer A sends: [Msg1: 10 bytes] and [Msg2: 20 bytes]. Peer B might receive:
    // Packet 1: [Msg1] + [First 5 bytes of Msg2]
    // Packet 2: [Rest of Msg2]
    void Connection::OnReadyRead() {
        if (!socket) return;
        try {
            // 1. Grab whatever data is currently on the wire (could be half a message)
            Buffer chunk = socket->Receive(8192);
            if (chunk.empty()){
                socket=nullptr;
                return;
            }
            // 2. Append it to our 'stomach' (the buffer)
            recv_buffer.insert(recv_buffer.end(), chunk.begin(), chunk.end());
            // 3. Try to digest it
            ProcessBuffer();
        } catch (...) {
            socket = nullptr;
        }
    }

    void Connection::ProcessBuffer() {
        while (true) {
            // --- HANDSHAKE ---
            if (state == HANDSHAKING) {
                if (recv_buffer.size() < 68) break; 
                
                recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 68);
                handshake_done = true;
                state = DOWNLOADING;
                // std::cout << "[Conn] Handshake OK! Sent Interested to " << peer.ip << std::endl;
                socket->Send(Message::BuildInterested());
                continue; 
            }

            // --- MESSAGES ---
            if (recv_buffer.size() < 4) break;
            uint32_t len = Message::ReadMessageLength(recv_buffer);

            if (len == 0) { // Keep-Alive
                recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4);
                continue;
            }

            if (recv_buffer.size() < 4 + len) break;

            uint8_t id = recv_buffer[4]; 
            Buffer payload;
            if (len > 1) {
                payload.insert(payload.end(), recv_buffer.begin() + 5, recv_buffer.begin() + 4 + len);
            }
            
            recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4 + len);
            switch (id) {
                // The peer is saying: "I have bandwidth. Ask me for data."
                case Message::UNCHOKE:
                    // std::cout << "[Conn] Peer UNCHOKED us! Requesting blocks...\n";
                    choked = false;
                    RequestNextBlock(); 
                    break;
                // case Message::BITFIELD:
                    // Peers often send this first. Good to know we got it.
                    // std::cout << "[Conn] Received Bitfield from peer." << std::endl;
                    // break;
                case Message::CHOKE:
                    choked = true;
                    break;
                case Message::PIECE: {
                    // The peer sent us actual file data.
                    uint32_t index =BufferUtils::ReadBE32(payload, 0);
                    uint32_t begin =BufferUtils::ReadBE32(payload, 4);
                    Buffer data(payload.begin() + 8, payload.end());
                    
                    downloader.OnBlockReceived(index, begin, data);
                    RequestNextBlock(); 
                    break;
                }
            }
        }
    }
    // Piece: Large chunk (e.g., 256 KB). Tracks correctness (Hash).
    // Block: Network chunk (Standard is 16 KB).
    void Connection::RequestNextBlock() {
        if (choked) return;
        // 1. Do we need a new Piece?
        if (current_piece == -1) {
            current_piece = downloader.GetNextPieceToRequest();
            block_offset = 0; // Start at byte 0 of this piece
            if (current_piece == -1) {
                std::cout << "[Conn] Download Complete!" << std::endl;
                return;
            }
        }
        long long piece_len = downloader.torrent.piece_length;
        if (current_piece == downloader.torrent.piece_hashes.size() - 1) {
            long long rem = downloader.torrent.length % piece_len;
            if(rem != 0) piece_len = rem;
        }
        
        if (block_offset < piece_len) {
            // 2. Calculate Block Size
            // Usually 16384 bytes, but the last block might be smaller.
            uint32_t req_len = 16384;
            if (block_offset + req_len > piece_len) req_len = piece_len - block_offset;
            // 3. Send the Request
            // "Send me Piece #5, starting at offset 0, for 16384 bytes"
            socket->Send(Message::BuildRequest(current_piece, block_offset, req_len));
            // 4. Advance offset
            block_offset += req_len;
        } 
        
        if (block_offset >= piece_len) {
            current_piece = -1; 
        }
    }
}
// Summary of the Flow
// Epoll says: "Connected!" --> We send Handshake.
// Epoll says: "Data In!" --> We read Handshake --> Send Interested.
// Epoll says: "Data In!" --> We read UNCHOKE --> Send Request (Piece 0, Offset 0).
// Epoll says: "Data In!" --> We read PIECE (0, 0) --> Write to Disk --> Send Request (Piece 0, Offset 16KB).
// Repeat until the piece is done, then ask Downloader for the next Piece.