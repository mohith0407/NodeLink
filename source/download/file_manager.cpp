#include "download/file_manager.h"
#include <stdexcept>
#include <iostream>

namespace BitTorrent {

    FileManager::FileManager(const std::string& fname, long long total_size) : filename(fname) {
        // Open file for Read/Write + Binary
        file_stream.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file_stream.is_open()) {
            // Try creating it if it doesn't exist
            file_stream.open(filename, std::ios::out | std::ios::binary);
        }
        
        if (!file_stream) throw std::runtime_error("Could not open file for writing: " + filename);

        // Pre-allocate space (Optimization)
        try {
            file_stream.seekp(total_size - 1);
            file_stream.write("", 1);
            file_stream.flush();
        } catch(...) {
            std::cerr << "Warning: Could not pre-allocate file space." << std::endl;
        }
    }

    FileManager::~FileManager() {
        if (file_stream.is_open()) file_stream.close();
    }

    void FileManager::WriteBlock(uint32_t piece_index, uint32_t offset_in_file, const Buffer& data) {
        std::lock_guard<std::mutex> lock(file_mutex); // Lock the file
        
        file_stream.seekp(offset_in_file);
        file_stream.write(reinterpret_cast<const char*>(data.data()), data.size());
        
        // Optional: Flush occasionally, but not every time for speed
    }
}