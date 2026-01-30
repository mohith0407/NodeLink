#pragma once
#include "download/Connection.h"
#include <vector>
#include <memory>
#include <functional>
#include <sys/epoll.h>

namespace BitTorrent {

    class Farm {
        int epfd;
        std::vector<std::shared_ptr<Connection>> connections;
        struct epoll_event events[64]; // Max events to process per loop

    public:
        Farm();
        ~Farm();
        void AddConnection(std::shared_ptr<Connection> conn);
        void Run(); // The main loop
        void Run(std::function<bool()> checkComplete);
    };
}