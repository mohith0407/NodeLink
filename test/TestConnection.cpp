#include "download/Connection.h"
#include "download/Downloader.h"
#include "tracker/Tracker.h"
#include <iostream>
#include <unistd.h>
#include <poll.h>

int main(int argc, char* argv[]) {
    if(argc < 2) { std::cout << "Usage: ./test_connection <file.torrent>\n"; return 1; }

    try {
        using namespace BitTorrent;
        std::cout << "[Test] Loading Torrent...\n";
        TorrentFile tf = TorrentFile::Load(argv[1]);
        std::string my_id = "-CPP-TEST-CLIENT-001"; 

        std::cout << "[Test] Contacting Tracker...\n";
        std::vector<Peer> peers = Tracker::GetPeers(tf, my_id);
        std::cout << "[Test] Found " << peers.size() << " peers.\n";

        if(peers.empty()) return 1;

        // Create a Downloader (Dependencies: Worker components already tested)
        Downloader d(tf, my_id, {}); 
        
        int attempts = 0;
        for(const auto& p : peers) {
            if(attempts++ > 5) break; 

            std::cout << "[Test] Connecting to " << p.ip << ":" << p.port << "... ";
            Connection conn(p, d);
            conn.Connect();

            if(conn.GetSocketFd() == -1) {
                std::cout << "Failed (Socket Error)\n";
                continue;
            }

            // Mini Event Loop
            struct pollfd pfd;
            pfd.fd = conn.GetSocketFd();
            pfd.events = POLLIN | POLLOUT;

            auto start = std::chrono::steady_clock::now();
            bool handshook = false;

            while(true) {
                if(std::chrono::steady_clock::now() - start > std::chrono::seconds(3)) break;

                int ret = poll(&pfd, 1, 500); 
                if(ret > 0) {
                    if(pfd.revents & POLLOUT) conn.OnReadyWrite();
                    if(pfd.revents & POLLIN)  conn.OnReadyRead();
                }

                if(conn.handshake_done) {
                    handshook = true;
                    break;
                }
            }

            if(handshook) {
                std::cout << "SUCCESS! Handshake completed.\n";
                std::cout << "[PASS] Network Layer is working.\n";
                return 0; 
            } else {
                std::cout << "Timeout.\n";
            }
        }
        
        std::cout << "[FAIL] Could not handshake with any peer.\n";

    } catch(std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}