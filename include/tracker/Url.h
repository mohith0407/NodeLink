#pragma once
#include <string>
#include <cstdint>

namespace BitTorrent {

    struct Url {
        std::string protocol; // "http" or "udp"
        std::string host;     // "tracker.debian.org"
        int port;             // 6969
        std::string path;     // "/announce"

        // parse string -> Url Object
        static Url Parse(const std::string& url_str);
    };
}