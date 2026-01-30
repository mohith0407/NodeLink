#include "download/Worker.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>

using namespace std;

namespace BitTorrent {

    // --- WRITER IMPLEMENTATION ---
    Writer::Writer(string filename) : out(filename, ios::binary | ios::out) {}

    Writer::~Writer() {
        { lock_guard<mutex> lk(mx); stop_flag = true; }
        cv.notify_one();
        if(t.joinable()) t.join();
    }

    void Writer::start() {
        t = thread([this](){
            while(true) {
                unique_lock<mutex> lock(mx);
                cv.wait(lock, [this]{ return !q.empty() || stop_flag; });

                while(!q.empty()) {
                    Job j = move(q.front());
                    q.pop();
                    lock.unlock();
                    
                    // Actual File Write
                    out.seekp(j.offset);
                    out.write((char*)j.b.data(), j.b.size());
                    
                    lock.lock();
                }
                if(stop_flag && q.empty()) break;
            }
        });
    }

    void Writer::add(Buffer& b, int64_t offset) {
        lock_guard<mutex> lk(mx);
        q.push(Job(move(b), offset)); // Moves data to queue
        cv.notify_one();
    }

    // --- SPEED / UI IMPLEMENTATION ---
    Speed::Speed(size_t total, string name) : total_bytes(total), filename(name) {}
    Speed::~Speed() { 
        stop_flag = true; 
        if(t.joinable()) t.join(); 
        cout << "\033[?25h"; // Show Cursor again
        cout << "\n";
    }

    string Speed::FormatBytes(double bytes) {
        const char* units[] = {"B", "KB", "MB", "GB"};
        int i=0;
        while(bytes >= 1024 && i < 3) { bytes /= 1024; i++; }
        stringstream ss; ss << fixed << setprecision(2) << bytes << " " << units[i];
        return ss.str();
    }

    void Speed::add(size_t b) {
        bytes_downloaded += b;
        session_bytes += b;
    }

    void Speed::start() {
        // Clear screen and hide cursor
        cout << "\033[2J\033[1;1H"; 
        cout << "\033[?25l"; // Hide Cursor
        
        t = thread([this](){
            int spin_index = 0;
            const char spinner[] = {'|', '/', '-', '\\'};

            while(!stop_flag) {
                // 1. Calculate Speed & Progress
                size_t current = session_bytes.exchange(0);
                double speed = current * 5.0; // 200ms refresh -> x5 for per second

                double progress = total_bytes > 0 ? (double)bytes_downloaded / total_bytes : 0;
                int percent = (int)(progress * 100);
                if (percent > 100) percent = 100;

                // 2. Prepare Bar Dimensions
                int bar_width = 50;
                int pos = bar_width * progress;
                if(pos > bar_width) pos = bar_width;

                // 3. Move Cursor Home (Don't clear whole screen, just overwrite)
                cout << "\033[H"; 

                // --- LINE 1: Filename & Status ---
                cout << "\033[1;36m[ Downloading ]\033[0m " << filename << "\033[K\n"; 

                // --- LINE 2: Progress Bar & Spinner ---
                cout << "\033[1;33m["; // Yellow Color
                for(int i=0; i<bar_width; ++i) {
                    if (i < pos) cout << "#";      // Filled
                    else if (i == pos) cout << ">"; // Tip
                    else cout << ".";              // Empty
                }
                cout << "]\033[0m " 
                     << "\033[1;37m" << percent << "%\033[0m " 
                     << "\033[1;35m" << spinner[spin_index] << "\033[0m" // Purple Spinner
                     << "\033[K\n";

                // --- LINE 3: Stats ---
                cout << "\033[1;32mSpeed: \033[0m" << left << setw(12) << (FormatBytes(speed) + "/s")
                     << " \033[1;34mDownloaded: \033[0m" << FormatBytes(bytes_downloaded) 
                     << " / " << FormatBytes(total_bytes) 
                     << "\033[K\n"; // Clear rest of line

                // Rotate Spinner
                spin_index = (spin_index + 1) % 4;
                
                this_thread::sleep_for(chrono::milliseconds(200));
            }
        });
    }

}


// concepts to read and description of above code
// 1. The Writer (Writer Class)
// Role: The "Disk Manager." Problem: Writing to a hard drive is slow (milliseconds). Receiving data from RAM is fast (microseconds). If the network thread writes to disk directly, it "blocks" and loses network speed.

// The Solution:
// The Queue (std::queue<Job> q): A buffer between the Network and the Disk.
// The Thread (t): A dedicated worker who does nothing but watch this queue.

// The Logic:
// Producer (Network): Calls writer.add(). It throws the data into the queue and returns immediately. Time taken: ~0s.
// Consumer (Thread): Wakes up, sees data in the queue, and writes it to disk. Time taken: ~10ms.


// mutex (Mutual Exclusion): Ensures the Network thread and Disk thread don't touch the queue at the same time (which would crash the app).

// condition_variable (cv): This is a "Sleep" button.
// If queue is empty: The thread sleeps (consuming 0% CPU).
// If data arrives: cv.notify_one() wakes the thread up.

// 2. The Speed/UI (Speed Class)
// Role: The "Dashboard." Problem: std::cout is actually very slow. If you print every time a packet arrives, your terminal will flicker and your download will slow down.

// The Solution:

// Atomic Counters: bytes_downloaded is an std::atomic. This allows the Network thread to increment it safely without locking a mutex (very fast).

// Refresh Loop: The UI thread sleeps for 200ms, wakes up, calculates the speed, draws the bar, and sleeps again.

// ANSI Escape Codes: \033[H (Home) and \033[2J (Clear). These allow us to overwrite the previous text instead of printing a new line, creating the animation effect.