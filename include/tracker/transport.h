#pragma once
#include "parsing/Buffer.h"
#include <string>
#include <vector>

namespace BitTorrent {

    // Base Class for Networking
    class Transport {
    protected:
        int sock = -1;
        std::string host;
        int port;

    public:
        Transport(const std::string& h, int p) : host(h), port(p) {}
        virtual ~Transport(); // Closes socket automatically

        virtual void Send(const Buffer& data) = 0;
        virtual Buffer Receive(size_t buffer_size = 4096) = 0;
        void SetTimeout(int seconds);

        int GetSocket() const { return sock; }
    };

    class TcpClient : public Transport {
    public:
        TcpClient(const std::string& host, int port);
        void Send(const Buffer& data) override;
        Buffer Receive(size_t buffer_size = 4096) override;
    };

    class UdpClient : public Transport {
    public:
        UdpClient(const std::string& host, int port);
        void Send(const Buffer& data) override;
        Buffer Receive(size_t buffer_size = 4096) override;
    };
}