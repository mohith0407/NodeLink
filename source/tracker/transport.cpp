#include "tracker/transport.h"
#include <sys/socket.h>
#include <netdb.h> 
#include <unistd.h> 
#include <cstring>
#include <iostream>
#include <arpa/inet.h>

namespace BitTorrent {

    Transport::Transport(const std::string& host, int port, int type) {
        // 1. DNS Lookup (Turn "tracker.opentrackr.org" into "123.45.67.89")
        struct addrinfo hints{}, *res;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET; // IPv4
        hints.ai_socktype = type;

        std::string port_str = std::to_string(port);
        int status = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
        if (status != 0) {
            throw std::runtime_error("DNS Lookup failed: " + std::string(gai_strerror(status)));
        }

        // 2. Create the Socket (The Phone)
        sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock_fd < 0) {
            freeaddrinfo(res);
            throw std::runtime_error("Socket creation failed");
        }

        // 3. Connect (Dial the number)
        if (connect(sock_fd, res->ai_addr, res->ai_addrlen) < 0) {
            freeaddrinfo(res);
            close(sock_fd);
            throw std::runtime_error("Connection failed");
        }

        freeaddrinfo(res);
        is_connected = true;
    }

    Transport::~Transport() {
        if (sock_fd >= 0) close(sock_fd);
    }

    void Transport::Send(const Buffer& data) {
        if (data.empty()) return;
        if (::send(sock_fd, data.data(), data.size(), 0) < 0) 
            throw std::runtime_error("Send failed");
    }

    Buffer Transport::Receive(size_t buffer_size) {
        Buffer buf(buffer_size);
        ssize_t received = ::recv(sock_fd, buf.data(), buffer_size, 0);
        
        if (received < 0) throw std::runtime_error("Receive failed (Timeout?)");
        
        buf.resize(received);
        return buf;
    }
    
    void Transport::SetTimeout(int seconds) {
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;
        setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    }

    TcpClient::TcpClient(const std::string& host, int port) 
        : Transport(host, port, SOCK_STREAM) {}

    UdpClient::UdpClient(const std::string& host, int port) 
        : Transport(host, port, SOCK_DGRAM) {}
}