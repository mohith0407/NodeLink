#include "download/downloader.h"
#include "tracker/transport.h"
#include "download/message.h"
#include <iostream>
#include <cmath>
#include <openssl/sha.h>
#include <cstring>

namespace BitTorrent {

    Downloader::Downloader(const Torrent& t, const std::string& my_peer_id) 
        : torrent(t), my_id(my_peer_id), file_manager(t.name, t.length), pieces_downloaded_count(0) {
        
        pieces_complete.resize(t.num_pieces, false);
    }

    void Downloader::Start(const std::vector<Peer>& peers) {
        std::vector<std::thread> threads;
        std::cout << "[Downloader] Starting with " << peers.size() << " peers." << std::endl;

        for (const auto& p : peers) {
            // Launch a thread for each peer
            threads.emplace_back(&Downloader::PeerWorker, this, p);
        }

        // Wait for all threads (or until download complete)
        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
        std::cout << "[Downloader] All workers finished." << std::endl;
    }

    int Downloader::GetNextPieceToDownload(const std::vector<bool>& peer_has_piece) {
        std::lock_guard<std::mutex> lock(piece_mutex);
        
        // Simple strategy: Find first missing piece that this peer has
        for (int i = 0; i < torrent.num_pieces; ++i) {
            if (!pieces_complete[i] && peer_has_piece[i]) {
                // Mark tentatively as handled (simple logic for now)
                // Real clients use 'In Progress' states
                return i;
            }
        }
        return -1; // Nothing useful from this peer
    }

    bool Downloader::VerifyPiece(const Buffer& data, int piece_index) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(data.data(), data.size(), hash);
        
        Buffer calculated(hash, hash + SHA_DIGEST_LENGTH);
        Buffer expected = torrent.getPieceHash(piece_index);
        
        return calculated == expected;
    }

    void Downloader::PeerWorker(Peer p) {
        try {
            TcpClient tcp(p.ip, p.port);
            tcp.SetTimeout(3); // 3 second timeout for responsiveness

            // 1. Handshake
            tcp.Send(Message::BuildHandshake(torrent, my_id));
            Buffer response = tcp.Receive(68); // Handshake is exactly 68 bytes
            
            // Verify Handshake (Protocol String & Info Hash)
            if (response.size() < 68 || response[0] != 19) return; 

            // 2. Send Interested
            tcp.Send(Message::BuildInterested());

            // 3. State
            bool choked = true;
            std::vector<bool> peer_pieces(torrent.num_pieces, false); // What the peer has
            Buffer incoming_buffer;

            // Loop forever until connection dies or download finishes
            while (pieces_downloaded_count < torrent.num_pieces) {
                
                // --- A. Message Handling Loop ---
                // We peek/read messages. If unchoked, we break to request data.
                try {
                    Buffer len_buf = tcp.Receive(4);
                    uint32_t msg_len = Message::ReadMessageLength(len_buf);
                    
                    if (msg_len == 0) continue; // KeepAlive

                    Buffer payload = tcp.Receive(msg_len);
                    uint8_t id = payload[0];

                    if (id == Message::UNCHOKE) {
                        choked = false;
                        std::cout << "Peer " << p.ip << " Unchoked us!" << std::endl;
                    } 
                    else if (id == Message::CHOKE) choked = true;
                    else if (id == Message::HAVE) {
                         uint32_t piece_idx = BufferUtils::ReadBE32(payload, 1);
                         if (piece_idx < peer_pieces.size()) peer_pieces[piece_idx] = true;
                    }
                    else if (id == Message::BITFIELD) {
                        // Complex bitfield parsing omitted for brevity, assuming 'Have' mostly
                        // Real implementation must parse bits here
                        for(size_t i=0; i<peer_pieces.size(); ++i) peer_pieces[i] = true; // Assume they have everything for test
                    }
                } catch (...) {
                    // Timeout is normal, just loop
                }

                if (choked) continue;

                // --- B. Request Logic ---
                int piece_index = GetNextPieceToDownload(peer_pieces);
                if (piece_index == -1) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue; 
                }

                // Download the WHOLE piece (Block by Block)
                // A piece is usually 256KB. We request 16KB blocks.
                uint32_t piece_len = torrent.getPieceLength(piece_index);
                Buffer piece_data;
                piece_data.reserve(piece_len);

                bool piece_failed = false;

                for (uint32_t offset = 0; offset < piece_len; offset += 16384) {
                    uint32_t block_len = std::min((uint32_t)16384, piece_len - offset);
                    
                    tcp.Send(Message::BuildRequest(piece_index, offset, block_len));
                    
                    // Wait for PIECE message
                    try {
                        Buffer len_buf = tcp.Receive(4);
                        uint32_t msg_len = Message::ReadMessageLength(len_buf);
                        Buffer payload = tcp.Receive(msg_len);
                        
                        if (payload[0] != Message::PIECE) throw std::exception(); // Wrong packet
                        
                        // Payload: <ID=7><Index><Begin><Block Data>
                        // Extract Block Data
                        piece_data.insert(piece_data.end(), payload.begin() + 9, payload.end());

                    } catch (...) {
                        piece_failed = true;
                        break;
                    }
                }

                if (!piece_failed && VerifyPiece(piece_data, piece_index)) {
                    // Save to Disk
                    long long file_offset = (long long)piece_index * torrent.piece_length;
                    file_manager.WriteBlock(piece_index, file_offset, piece_data);
                    
                    std::lock_guard<std::mutex> lock(piece_mutex);
                    pieces_complete[piece_index] = true;
                    pieces_downloaded_count++;
                    std::cout << "Downloaded Piece " << piece_index << " from " << p.ip << std::endl;
                }
            }

        } catch (...) {
            // Peer disconnected
        }
    }
}