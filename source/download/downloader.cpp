#include "download/Downloader.h"
#include "download/Connection.h"
#include "download/Farm.h"
#include <iostream>
#include <algorithm>

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
        // 1. Calculate where in the file this goes
        int64_t global_offset = (int64_t)piece_index * torrent.piece_length + offset;
        
        // 2. Update Speed UI
        s.add(data.size());
        
        // 3. Send to Disk Writer
        // Note: Writer::add() moves the buffer, so data is invalid after this
        file_writer.add(data, global_offset);
        downloaded_bytes += data.size();
    }
}


// Shared Pointers (std::shared_ptr)
// Problem: If we used Connection* c = new Connection(), we would have to manually delete it later. If the program crashes or exits early, we leak memory.

// Solution: shared_ptr is a "smart" pointer. It counts how many people are holding it. When the Downloader dies and the Farm dies, the pointer count hits 0, and the Connection cleans itself up automatically.