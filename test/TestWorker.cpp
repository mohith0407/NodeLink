#include "download/Worker.h"
#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include <cassert>

// Usage: ./step2_test_worker

int main() {
    std::cout << "[Test] Starting Worker Test (Writer + Speed UI)...\n";
    
    std::string filename = "test_output.bin";
    size_t total_size = 1024 * 1024 * 10; // 10 MB Fake File
    
    // 1. Initialize Components
    BitTorrent::Writer writer(filename);
    BitTorrent::Speed speed(total_size, "test_file.iso");

    writer.start();
    speed.start();

    // 2. Simulate Download Loop
    // We will write 10 chunks of 1MB each
    size_t chunk_size = 1024 * 1024;
    BitTorrent::Buffer junk_data(chunk_size, 'A'); // 1MB of 'A's

    for(int i=0; i<10; ++i) {
        // Create a copy because writer.add() moves the data
        BitTorrent::Buffer chunk_copy = junk_data;
        
        // Add to Writer
        writer.add(chunk_copy, i * chunk_size);
        
        // Update Speed
        speed.add(chunk_size);
        
        // Sleep to simulate network delay and let UI update
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    // 3. Stop
    // Destructors will handle stop(), but let's be explicit to test clean exit
    // Note: In real code, we just let them go out of scope.
    
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Show 100% for a second
    std::cout << "\n[Test] Simulation done. Verifying file...\n";
    
    // 4. Verify File Content
    std::ifstream in(filename, std::ios::binary | std::ios::ate);
    if (!in.is_open()) {
        std::cerr << "[FAIL] Could not open output file!\n";
        return 1;
    }
    
    size_t written_size = in.tellg();
    if (written_size == total_size) {
        std::cout << "[PASS] File size matches (" << written_size << " bytes).\n";
    } else {
        std::cerr << "[FAIL] File size mismatch! Expected " << total_size << ", Got " << written_size << "\n";
        return 1;
    }

    // Cleanup
    std::remove(filename.c_str());
    return 0;
}