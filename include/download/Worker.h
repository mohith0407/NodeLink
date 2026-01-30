#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <string>
#include <atomic>
#include "parsing/Buffer.h"

namespace BitTorrent {

    struct Job {
        Buffer b;
        int64_t offset;
        Job(Buffer b, int64_t offset): b(std::move(b)), offset(offset) {}
    };

    class Writer {
        std::thread t;
        std::mutex mx;
        std::condition_variable cv;
        std::queue<Job> q;
        std::ofstream out;
        bool stop_flag = false;
    public:
        Writer(std::string filename);
        ~Writer(); 
        void start();
        void add(Buffer& b, int64_t offset); // Note: Non-const ref for move semantics
    };

    class Speed {
        std::thread t;
        std::atomic<size_t> bytes_downloaded{0};
        std::atomic<size_t> total_bytes{0};
        std::atomic<size_t> session_bytes{0};
        std::string filename;
        bool stop_flag = false;
        
        std::string FormatBytes(double bytes);
    public:
        Speed(size_t total, std::string name);
        ~Speed();
        void start();
        void add(size_t bytes);
    };
}