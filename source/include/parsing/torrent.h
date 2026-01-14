#ifndef TORRENT_H
#define TORRENT_H
#include "parsing/buffer.h"
#include "parsing/bencode.h"
#include "tracker/url.h"
#include<string>
using namespace std;
class torrent{
    buffer get_bytes(const string& filename);
    buffer get_hash_info(const bencode::item& item);
    long long get_length(const becode::item& item);
    public:
        buffer info_hash;
        long long length;
        url_t url;
        string name;
        unsigned int piece_length;
        unsigned int pieces;
        torrent(const string& filename);
        unsigned int get_piece_length(unsigned int piece);
        unsigned int get_n_blocks(unsigned int piece);
        unsigned int get_block_length(unsigned int piece, unsigned int block_index);
        // 2^14
        static const unsigned int BLOCK_SIZE = (1<<14);


};
#endif