#include "download/Farm.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <stdexcept>

namespace BitTorrent {

    Farm::Farm() {
        // 1. Create the epoll instance
        epfd = epoll_create1(0);
        if(epfd < 0) throw std::runtime_error("Failed to create epoll");
    }

    Farm::~Farm() {
        if(epfd >= 0) close(epfd);
    }

    void Farm::AddConnection(std::shared_ptr<Connection> conn) {
        // Start the TCP connection
        conn->Connect();
        
        if(conn->GetSocketFd() >= 0) { 
            struct epoll_event ev;
            // LISTEN for READ (Data in) and WRITE (Ready to send/Connect success)
            ev.events = EPOLLIN | EPOLLOUT; 
            ev.data.ptr = conn.get(); // Store pointer to retrieve later
            
            // Add to epoll
            if(epoll_ctl(epfd, EPOLL_CTL_ADD, conn->GetSocketFd(), &ev) < 0) {
                std::cerr << "[Farm] Failed to add socket to epoll\n";
            } else {
                connections.push_back(conn);
            }
        }
    }

    void Farm::Run(std::function<bool()> checkComplete) {
        while(!connections.empty()) {
            // 1. CHECK IF DOWNLOAD IS FINISHED
            if (checkComplete()) {
                // std::cout << "[Farm] Download limit reached. Stopping.\n";
                break;
            }
            // 2. Wait for events (Blocking)
            int nfds = epoll_wait(epfd, events, 64, 1000); // 1 sec timeout
            
            if(nfds < 0) {
                if(errno == EINTR) continue;
                break; // Error
            }

            for(int i=0; i<nfds; ++i) {
                Connection* conn = static_cast<Connection*>(events[i].data.ptr);
                
                // --- HANDLING EVENTS ---
                
                // 1. Connection established OR Space available to write
                if(events[i].events & EPOLLOUT) {
                    conn->OnReadyWrite();
                }

                // 2. Data arrived from peer
                if(events[i].events & EPOLLIN) {
                    conn->OnReadyRead();
                }
                
                // 3. Errors / Hangups
                if(events[i].events & (EPOLLERR | EPOLLHUP)) {
                    // We can handle disconnection cleanup here if we want
                    // For now, Connection class handles socket closure safely
                }
            }
            
            // Cleanup closed connections occasionally (Simplification)
            // In a pro client, we'd remove them from the vector here.
        }

    }
    void Farm::Run() {
        // Delegate to the other run with a "never stop" condition
        Run([](){ return false; });
    }

}

// Role: The "Traffic Controller." Problem: We have 5, 50, or 500 connections. We cannot use cin >> or socket->read() on them because those functions block (freeze) until data arrives. If we wait for Peer 1, we ignore Peer 2.

// The Solution: I/O Multiplexing (epoll) Instead of asking each socket "Do you have data?", we tell the Operating System (Linux Kernel):

// "Here is a list of 50 sockets. Put me to sleep. Wake me up ONLY if one of them does something."