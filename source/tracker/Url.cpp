#include "tracker/Url.h"
#include <stdexcept>
#include <algorithm>

namespace BitTorrent {

    Url Url::Parse(const std::string& url_str) {
        Url res;
        std::string s = url_str;

        // 1. Extract Protocol
        std::string delim = "://";
        size_t pos = s.find(delim);
        if (pos == std::string::npos) throw std::runtime_error("Invalid URL: No protocol");

        res.protocol = s.substr(0, pos);
        s.erase(0, pos + delim.length());

        // 2. Extract Path (everything after the first '/')
        pos = s.find('/');
        if (pos != std::string::npos) {
            res.path = s.substr(pos); // e.g., "/announce"
            s = s.substr(0, pos);     // Remaining: "host:port"
        } else {
            res.path = "/";
        }

        // 3. Extract Port (everything after ':')
        pos = s.find(':');
        if (pos != std::string::npos) {
            res.port = std::stoi(s.substr(pos + 1));
            res.host = s.substr(0, pos);
        } else {
            res.host = s;
            // Default ports
            if (res.protocol == "http") res.port = 80;
            else if (res.protocol == "udp") res.port = 80; // UDP trackers vary, but default exists
            else res.port = 6969; // Common tracker port
        }

        return res;
    }
}