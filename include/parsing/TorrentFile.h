#pragma once

#include "Bnode.h"
#include <string>
#include <vector>

namespace BitTorrent {

    struct TorrentFile {
        std::string announce;
        std::string name;
        int64_t length;
        int64_t piece_length;
        std::vector<std::string> piece_hashes;
        Buffer info_hash; // The Unique ID (20 bytes)

        // The only function you need: Load from disk
        static TorrentFile Load(const std::string& filepath);
    };

}