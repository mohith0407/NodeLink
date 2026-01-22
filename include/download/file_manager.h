#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "parsing/buffer.h"
#include <string>
#include <fstream>
#include <mutex>

namespace BitTorrent {

    class FileManager {
    public:
        FileManager(const std::string& filename, long long total_size);
        ~FileManager();

        // Writes data to a specific position in the file safely
        void WriteBlock(uint32_t piece_index, uint32_t block_offset, const Buffer& data);

    private:
        std::string filename;
        std::ofstream file_stream;
        std::mutex file_mutex; // The lock that ensures safety
    };
}
#endif