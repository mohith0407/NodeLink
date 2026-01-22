#include "parsing/torrent.h"
#include <fstream>
#include <openssl/sha.h>
#include <stdexcept>
#include <cmath>

namespace BitTorrent {

    Torrent::Torrent(const std::string& filename) {
        Buffer fileData = readFile(filename);
        Bencode::Item root = Bencode::Parse(fileData);

        this->tracker_url = root.getStringFromDict("announce");
        this->info_hash = calculateInfoHash(root);

        Bencode::Item info = root.getFromDict("info");
        this->name = info.getStringFromDict("name");
        this->piece_length = info.getIntFromDict("piece length");
        
        // Handle Single File Mode
        if (info.hasKey("length")) {
            this->length = info.getIntFromDict("length");
        } else {
             throw std::runtime_error("Multi-file torrents not supported yet");
        }

        this->pieces_blob = info.getBufferFromDict("pieces");
        this->num_pieces = pieces_blob.size() / 20;
    }

    Buffer Torrent::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) throw std::runtime_error("Cannot open file: " + filename);
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        Buffer buffer(size);
        if (!file.read((char*)buffer.data(), size)) throw std::runtime_error("Read failed");
        return buffer;
    }

    Buffer Torrent::calculateInfoHash(const Bencode::Item& root) {
        Bencode::Item info = root.getFromDict("info");
        Buffer encoded = Bencode::Encode(info);
        
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(encoded.data(), encoded.size(), hash);
        return Buffer(hash, hash + SHA_DIGEST_LENGTH);
    }

    // Helper Math for Downloading
    uint32_t Torrent::getPieceLength(uint32_t idx) const {
        if (idx >= num_pieces) throw std::out_of_range("Invalid piece index");
        if (idx == num_pieces - 1) {
            uint32_t rem = length % piece_length;
            return (rem == 0) ? piece_length : rem;
        }
        return piece_length;
    }

    uint32_t Torrent::getBlocksPerPiece(uint32_t idx) const {
        uint32_t len = getPieceLength(idx);
        return (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    }

    uint32_t Torrent::getBlockLength(uint32_t p_idx, uint32_t b_idx) const {
        uint32_t p_len = getPieceLength(p_idx);
        uint32_t b_count = getBlocksPerPiece(p_idx);
        
        if (b_idx >= b_count) throw std::out_of_range("Invalid block index");
        if (b_idx == b_count - 1) {
            uint32_t rem = p_len % BLOCK_SIZE;
            return (rem == 0) ? BLOCK_SIZE : rem;
        }
        return BLOCK_SIZE;
    }

    Buffer Torrent::getPieceHash(uint32_t idx) const {
        if (idx >= num_pieces) throw std::out_of_range("Index error");
        auto start = pieces_blob.begin() + (idx * 20);
        return Buffer(start, start + 20);
    }
}