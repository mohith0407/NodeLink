#ifndef CONNECTION_H
#define CONNECTION_H

#include "parsing/torrent.h"
#include "tracker/tracker.h"
#include "parsing/buffer.h"
#include "tracker/tcp.h"
#include <vector>
#include "download/download.h"

class connection{
    private:
        void handle(buffer& message);
        void choke_handler();
        void unchoke_handler();
        void have_handler(buffer& payload);
        void bitfield_handler(buffer& payload);
        void piece_handler(buffer& payload);
        void enqueue(int index);
        void request_piece();

        buffer storage;
        const peer& remote;
        torrent& meta;
        bool is_choked;
        bool handshake_completed;
        bool is_connected;
        download& manager;
    public:
        tcp socket;
        connection(const peer& remote_peer,torrent& torrent_ref,download& download_ref);
        void ready();
};

#endif