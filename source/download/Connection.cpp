#include "download/Connection.h"
#include "download/Downloader.h"
#include "parsing/buffer.h"
#include "download/Message.h"
#include <iostream>

namespace BitTorrent
{

    // Local Helper
    // uint32_t ReadInt32(const Buffer& b, size_t offset) {
    //     if (b.size() < offset + 4) return 0;
    //     return (b[offset] << 24) | (b[offset+1] << 16) | (b[offset+2] << 8) | b[offset+3];
    // }

    Connection::Connection(Peer p, Downloader &d) : peer(p), downloader(d) {}

    void Connection::Connect()
    {
        try
        {
            socket = std::make_unique<TcpClient>(peer.ip, peer.port);
            socket->SetTimeout(2);
            state = CONNECTING;
        }
        catch (...)
        {
            socket = nullptr;
        }
    }

    void Connection::OnReadyWrite()
    {
        if (!socket)
            return;

        if (state == CONNECTING)
        {
            try
            {
                Buffer hs = Message::BuildHandshake(downloader.torrent, downloader.my_id);
                socket->Send(hs);
                state = HANDSHAKING;
                std::cout << "[Conn] Connected & Handshake sent to " << peer.ip << std::endl;
            }
            catch (...)
            {
                socket = nullptr;
            }
        }
    }
    // The Concept: TCP is a Stream, not a message queue. If Peer A sends: [Msg1: 10 bytes] and [Msg2: 20 bytes]. Peer B might receive:
    // Packet 1: [Msg1] + [First 5 bytes of Msg2]
    // Packet 2: [Rest of Msg2]
    void Connection::OnReadyRead()
    {
        if (!socket)
            return;
        try
        {
            // 1. Grab whatever data is currently on the wire (could be half a message)
            Buffer chunk = socket->Receive(8192);
            if (chunk.empty())
            {
                socket = nullptr;
                return;
            }
            // 2. Append it to our 'stomach' (the buffer)
            recv_buffer.insert(recv_buffer.end(), chunk.begin(), chunk.end());
            // 3. Try to digest it
            ProcessBuffer();
        }
        catch (const std::exception &e)
        {
            socket = nullptr;
            std::cerr << "[Conn] Error reading: " << e.what() << std::endl;
            socket = nullptr;
        }
    }

    void Connection::ProcessBuffer()
    {
        while (true)
        {
            // --- HANDSHAKE ---
            if (state == HANDSHAKING)
            {
                if (recv_buffer.size() < 68)
                    break;

                recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 68);
                handshake_done = true;
                state = DOWNLOADING;
                // std::cout << "[Conn] Handshake OK! Sent Interested to " << peer.ip << std::endl;
                socket->Send(Message::BuildInterested());
                continue;
            }

            // --- MESSAGES ---
            if (recv_buffer.size() < 4)
                break;
            uint32_t len = Message::ReadMessageLength(recv_buffer);

            if (len == 0)
            { // Keep-Alive
                recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4);
                continue;
            }

            if (recv_buffer.size() < 4 + len)
                break;

            uint8_t id = recv_buffer[4];
            Buffer payload;
            if (len > 1)
            {
                payload.insert(payload.end(), recv_buffer.begin() + 5, recv_buffer.begin() + 4 + len);
            }

            recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4 + len);
            switch (id)
            {
            // The peer is saying: "I have bandwidth. Ask me for data."
            case Message::UNCHOKE:
                // std::cout << "[Conn] Peer UNCHOKED us! Requesting blocks...\n";
                choked = false;
                RequestNextBlock();
                break;
            case Message::BITFIELD:
                peer_pieces.resize(downloader.torrent.piece_hashes.size(), false);
                for (size_t i = 0; i < payload.size(); ++i)
                {
                    uint8_t byte = payload[i];
                    for (int bit = 0; bit < 8; ++bit)
                    {
                        // Check bit 7 down to 0
                        if ((byte >> (7 - bit)) & 1)
                        {
                            size_t piece_index = i * 8 + bit;
                            if (piece_index < peer_pieces.size())
                            {
                                peer_pieces[piece_index] = true;
                            }
                        }
                    }
                }
                break;
            case Message::CHOKE:
                choked = true;
                break;
            case Message::PIECE:
            {
                // The peer sent us actual file data.
                uint32_t index = BufferUtils::ReadBE32(payload, 0);
                uint32_t begin = BufferUtils::ReadBE32(payload, 4);
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
    void Connection::RequestNextBlock()
    {
        if (choked)
            return;

        // 1. If we don't have a piece assigned, ask Downloader for one
        if (current_piece == -1)
        {
            // LOOP until we find a piece this peer actually has
            int attempts = 0;
            while (true)
            {
                int candidate = downloader.GetNextPieceToRequest();

                // If downloader returns -1, we are done with the whole file
                if (candidate == -1)
                {
                    std::cout << "[Conn] No more pieces to request." << std::endl;
                    return;
                }

                // CHECK: Does the peer have this piece?
                if (HasPiece(candidate))
                {
                    current_piece = candidate;
                    block_offset = 0;
                    break; // Found a good one!
                }
                else
                {
                    // Peer doesn't have it. In a real client, we put this back in the queue.
                    // For this simple version, we just skip it (which is inefficient but working).
                    // Ideally: downloader.PutPieceBack(candidate);

                    // Safety break to prevent infinite loops if peer has nothing
                    attempts++;
                    if (attempts > downloader.torrent.piece_hashes.size())
                        return;
                }
            }
        }

        // 2. Calculate Block Size (Existing logic is good)
        long long piece_len = downloader.torrent.piece_length;
        // Check if it's the last piece (might be smaller)
        if (current_piece == downloader.torrent.piece_hashes.size() - 1)
        {
            long long rem = downloader.torrent.length % piece_len;
            if (rem != 0)
                piece_len = rem;
        }

        if (block_offset < piece_len)
        {
            uint32_t req_len = 16384; // 16KB standard
            if (block_offset + req_len > piece_len)
                req_len = piece_len - block_offset;

            socket->Send(Message::BuildRequest(current_piece, block_offset, req_len));
            block_offset += req_len;
        }

        // 3. If piece is done, reset current_piece so we get a new one next time
        if (block_offset >= piece_len)
        {
            current_piece = -1;
            // OPTIONAL: Immediately request the next one to avoid latency gap
            // RequestNextBlock();
        }
    }

}
// Summary of the Flow
// Epoll says: "Connected!" --> We send Handshake.
// Epoll says: "Data In!" --> We read Handshake --> Send Interested.
// Epoll says: "Data In!" --> We read UNCHOKE --> Send Request (Piece 0, Offset 0).
// Epoll says: "Data In!" --> We read PIECE (0, 0) --> Write to Disk --> Send Request (Piece 0, Offset 16KB).
// Repeat until the piece is done, then ask Downloader for the next Piece.