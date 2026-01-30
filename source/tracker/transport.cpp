#include "tracker/Transport.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
// concepts to know
// In Unix/Linux, everything is a file. A text file is a file, your keyboard is a file, and a network connection is a file.

// In Code: int sock

// Explanation: When you create a connection, the OS gives you an integer ID (like 3, 4, 5).

// Writing to ID 3 sends data to the internet.

// Reading from ID 3 reads data from the internet.
namespace BitTorrent {

    Transport::~Transport() {
        if (sock >= 0) close(sock);
    }

    void Transport::SetTimeout(int seconds) {
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    }

    // --- TCP Implementation ---
    TcpClient::TcpClient(const std::string& host, int port) : Transport(host, port) {
        // 1. Resolve Hostname (DNS Lookup)
        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;       // IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP
        // "hints" tells the OS: "I want an IPv4 address (AF_INET) for TCP (SOCK_STREAM)"
        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
            throw std::runtime_error("Could not resolve host: " + host);
        }

        // 2. Create Socket
        // Returns a File Descriptor (e.g., 3)
        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0) throw std::runtime_error("Could not create TCP socket");

        // Step 3: Connect (The Handshake) -- THIS IS BLOCKING!
        // Sends SYN -> Waits for SYN-ACK -> Sends ACK.
        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
            close(sock);
            throw std::runtime_error("Could not connect to " + host);
        }

        freeaddrinfo(res);
    }

    void TcpClient::Send(const Buffer& data) {
        if (send(sock, data.data(), data.size(), 0) < 0) {
            throw std::runtime_error("TCP Send failed");
        }
    }

    Buffer TcpClient::Receive(size_t buffer_size) {
        Buffer res(buffer_size); // 1. Allocate a big bucket (e.g., 4096 bytes)
        // 2. Try to fill it
        ssize_t received = recv(sock, res.data(), buffer_size, 0);
        if (received < 0) throw std::runtime_error("TCP Receive failed");
        // If we asked for 4096 bytes but only got 50, we shrink the vector to size 50.
        res.resize(received); // Shrink to actual data size
        return res;
    }

    // --- UDP Implementation ---
    UdpClient::UdpClient(const std::string& host, int port) : Transport(host, port) {
        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;      // IPv4
        hints.ai_socktype = SOCK_DGRAM; // UDP

        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
            throw std::runtime_error("Could not resolve host: " + host);
        }

        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0) throw std::runtime_error("Could not create UDP socket");

        // For UDP, we 'connect' to set the default destination for send()
        // This makes the code simpler (no need for sendto every time)
        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
            throw std::runtime_error("UDP Connect failed");
        }
        freeaddrinfo(res);
    }

    void UdpClient::Send(const Buffer& data) {
        if (send(sock, data.data(), data.size(), 0) < 0) {
            throw std::runtime_error("UDP Send failed");
        }
    }

    Buffer UdpClient::Receive(size_t buffer_size) {
        Buffer res(buffer_size);
        ssize_t received = recv(sock, res.data(), buffer_size, 0);
        if (received < 0) throw std::runtime_error("UDP Receive failed");
        res.resize(received);
        return res;
    }
}

// Component,Task,Protocol,Class Used,Why?
// Tracker,"""Who has this file?""",UDP (mostly),UdpClient,"Fast, low overhead. We don't care if one packet is lost."
// Tracker,(If URL is http://),TCP,TcpClient,Compatibility with older web-based trackers.
// Peer,"""Send me Piece #45""",TCP,TcpClient,Must be perfect. No missing bytes allowed.
