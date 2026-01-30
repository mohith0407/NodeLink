#pragma once
#include "tracker/Peer.h"
#include "tracker/Transport.h" // Ensures we have TcpClient
#include "parsing/Buffer.h"
#include <memory>

namespace BitTorrent {

    class Downloader; // Forward declaration still useful here

    class Connection {
    public:
        enum State { CONNECTING, HANDSHAKING, DOWNLOADING };

        Peer peer;
        Downloader& downloader;
        std::unique_ptr<TcpClient> socket;
        State state = CONNECTING;
        
        bool choked = true;          
        bool handshake_done = false;

        // Data Buffering
        Buffer recv_buffer; 

        // Current Job
        int current_piece = -1;
        int block_offset = 0;

        Connection(Peer p, Downloader& d);
        
        void Connect();
        void OnReadyRead();  
        void OnReadyWrite(); 
        
        int GetSocketFd() { 
            return socket ? socket->GetSocket() : -1; 
        }

    private:
        void ProcessBuffer();   
        void RequestNextBlock(); 
    };
}