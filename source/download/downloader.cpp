#include "download/Downloader.h"
#include "download/Connection.h"
#include "download/Farm.h"
#include "parsing/Sha1.h"
#include <iostream>
#include <algorithm>
#include <cstring>
namespace BitTorrent {
    // Constructor (Ensure downloaded_bytes is initialized)
    Downloader::Downloader(const TorrentFile& tf, const std::string& id, const std::vector<Peer>& p_list)
        : torrent(tf), my_id(id), peers(p_list), 
          file_writer(tf.name), s(tf.length, tf.name) 
    {
        downloaded_bytes = 0; // Reset
    }
    void Downloader::Start() {
        std::cout << "[Downloader] Starting download for: " << torrent.name << std::endl;
        std::cout << "[Downloader] Total Size: " << torrent.length / (1024*1024) << " MB" << std::endl;
        std::cout << "Wait for atleast 2 minutes to see progress" <<std:: endl;
        // 1. Start Background Threads
        file_writer.start();
        s.start();

        // 2. Create Connections
        std::vector<std::shared_ptr<Connection>> conns;
        int count = 0;
        for(const auto& p : peers) {
             if(count >= 5) break; // Limit to 5 peers to avoid file descriptor limits
             std::cout << "[Downloader] Connecting to " << p.ip << "..." << std::endl;
             conns.push_back(std::make_shared<Connection>(p, *this));
             count++;
        }
        std::cout << "[Downloader] Initiating " << conns.size() << " connections..." << std::endl;

        // 3. Add to Farm
        Farm farm;
        for(auto& c : conns) farm.AddConnection(c);
        
        // 4. Run the Event Loop (Blocks here until done)
        // farm.Run();
        // lambda expression
        farm.Run([this]() {
            return this->IsComplete();
        });
        std::cout << "\n[Downloader] Download loop finished.\n";
    }

    void Downloader::OnBlockReceived(int piece_index, int offset, Buffer& data) {
        
        // 1. Initialize Buffer for New Piece
        if (piece_index != current_piece_index) {
            current_piece_index = piece_index;
            piece_bytes_received = 0;
            
            // Allocate memory for the full piece (usually 256KB)
            long long piece_len = torrent.piece_length;
            
            // Handle last piece case (it might be smaller)
            if (piece_index == torrent.piece_hashes.size() - 1) {
                long long rem = torrent.length % torrent.piece_length;
                if (rem != 0) piece_len = rem;
            }
            
            piece_buffer.assign(piece_len, 0); // Clear buffer
        }

        // 2. Copy Data to RAM Buffer
        if (offset + data.size() <= piece_buffer.size()) {
            std::memcpy(piece_buffer.data() + offset, data.data(), data.size());
            piece_bytes_received += data.size();
        }

        // 3. Update Speed UI (We still downloaded it, even if bad)
        s.add(data.size());

        // 4. CHECK INTEGRITY: Is the Piece Full?
        if (piece_bytes_received == piece_buffer.size()) {
            
            // A. Calculate Hash
            std::string calculated_hash = Sha1::Calculate(piece_buffer);
            
            // B. Get Expected Hash from Torrent File
            std::string expected_hash = torrent.piece_hashes[piece_index];

            // C. Compare
            if (calculated_hash == expected_hash) {
                // SUCCESS: Write to Disk
                int64_t global_offset = (int64_t)piece_index * torrent.piece_length;
                file_writer.add(piece_buffer, global_offset);
                
                downloaded_bytes += piece_buffer.size();
                // std::cout << "\r[Integrity] Piece " << piece_index << " Verified & Written.   " << std::flush;
            } else {
                // FAILURE: Discard
                std::cerr << "\n[Integrity] HASH MISMATCH on Piece " << piece_index << "! Discarding.\n";
                std::cerr << "Expected: " << expected_hash << "\n";
                std::cerr << "Got:      " << calculated_hash << "\n";
                
                // In Phase 2, we would re-queue this piece. 
                // For now, we just lose it (File will be incomplete).
            }
        }
    }
}


// Shared Pointers (std::shared_ptr)
// Problem: If we used Connection* c = new Connection(), we would have to manually delete it later. If the program crashes or exits early, we leak memory.

// Solution: shared_ptr is a "smart" pointer. It counts how many people are holding it. When the Downloader dies and the Farm dies, the pointer count hits 0, and the Connection cleans itself up automatically.