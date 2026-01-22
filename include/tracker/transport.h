#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "parsing/buffer.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <netinet/in.h>

namespace BitTorrent {

    // Abstract Base Class for Network Communication
    class Transport {
    protected:
        int sock_fd = -1; 
        bool is_connected = false;

    public:
        // Constructor opens the socket
        Transport(const std::string& host, int port, int type);
        
        // Destructor automatically closes the socket (RAII)
        virtual ~Transport();

        void Send(const Buffer& data);
        Buffer Receive(size_t buffer_size = 2048);
        
        // Essential for UDP: Don't wait forever if packets get lost
        void SetTimeout(int seconds);
    };

    // Specific implementation for TCP (Reliable, used for HTTP trackers)
    class TcpClient : public Transport {
    public:
        TcpClient(const std::string& host, int port);
    };

    // Specific implementation for UDP (Fast, used for standard Trackers)
    class UdpClient : public Transport {
    public:
        UdpClient(const std::string& host, int port);
    };
}
#endif