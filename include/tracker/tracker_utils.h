#ifndef TRACKER_UTILS_H
#define TRACKER_UTILS_H

#include "parsing/buffer.h"
#include <string>

namespace BitTorrent {

    struct Url {
        std::string full;
        std::string protocol;
        std::string host;
        int port;
        std::string path;

        static Url Parse(const std::string& url_str);
    };

    class HttpBuilder {
    public:
        // Encodes raw bytes to URL-safe string (e.g., Space -> %20)
        static std::string UrlEncodeToString(const Buffer& data);
        
        // Constructs the HTTP GET request for the tracker
        static Buffer BuildGetRequest(const Url& url, const Buffer& info_hash, const std::string& peer_id, int port);
    };
}
#endif