#ifndef TORRENT_H
#define TORRENT_H

#include "parsing/buffer.h"
#include "parsing/bencode.h"
#include <string>

namespace BitTorrent {

    class Torrent {
    public:
        Buffer info_hash;       // Unique ID of the torrent
        std::string tracker_url;
        std::string name;
        long long length;

        uint32_t piece_length;
        uint32_t num_pieces;
        static const uint32_t BLOCK_SIZE = 16 * 1024;

        // Constructor: Loads and parses the file
        explicit Torrent(const std::string& filename);

        // Download logic helpers
        uint32_t getPieceLength(uint32_t piece_index) const;
        uint32_t getBlocksPerPiece(uint32_t piece_index) const;
        uint32_t getBlockLength(uint32_t piece_index, uint32_t block_index) const;
        
        Buffer getPieceHash(uint32_t piece_index) const;

    private:
        Buffer pieces_blob; // Huge blob of concatenated hashes

        Buffer readFile(const std::string& filename);
        Buffer calculateInfoHash(const Bencode::Item& root);
    };
}
#endif