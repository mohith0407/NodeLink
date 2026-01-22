#include "tracker/tracker_utils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace BitTorrent {

    Url Url::Parse(const std::string& u) {
        Url res;
        res.full = u;
        std::string s = u;

        // 1. Protocol (udp:// or http://)
        std::string sep = "://";
        size_t pos = s.find(sep);
        if (pos == std::string::npos) throw std::runtime_error("Invalid URL: No protocol");
        
        res.protocol = s.substr(0, pos);
        s.erase(0, pos + sep.length());

        // 2. Path (starts at first /)
        pos = s.find('/');
        if (pos != std::string::npos) {
            res.path = s.substr(pos);
            s.erase(pos);
        } else {
            res.path = "/";
        }

        // 3. Port (optional, after :)
        pos = s.find(':');
        if (pos != std::string::npos) {
            res.port = std::stoi(s.substr(pos + 1));
            s.erase(pos);
        } else {
            res.port = (res.protocol == "http") ? 80 : 80;
        }

        res.host = s;
        return res;
    }

    std::string HttpBuilder::UrlEncodeToString(const Buffer& data) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (unsigned char c : data) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
                continue;
            }
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int(c);
            escaped << std::nouppercase;
        }
        return escaped.str();
    }

    Buffer HttpBuilder::BuildGetRequest(const Url& url, const Buffer& info_hash, const std::string& peer_id, int port) {
        std::ostringstream req;
        req << "GET " << url.path 
            << "?info_hash=" << UrlEncodeToString(info_hash)
            << "&peer_id=" << peer_id 
            << "&port=" << port
            << "&uploaded=0&downloaded=0&left=0&compact=1&event=started"
            << " HTTP/1.1\r\n";
        
        req << "Host: " << url.host << "\r\n";
        req << "Connection: Close\r\n\r\n"; // Double newline ends request

        std::string s = req.str();
        return Buffer(s.begin(), s.end());
    }
}