#include "tracker/tracker.h"
#include "tracker/transport.h"
#include "parsing/buffer.h"
#include <random>
#include <iostream>

namespace BitTorrent {

    // Magic Numbers defined by the BitTorrent Protocol (BEP 15)
    const uint64_t UDP_CONNECTION_MAGIC = 0x41727101980;
    const uint32_t ACTION_CONNECT = 0;
    const uint32_t ACTION_ANNOUNCE = 1;

    std::vector<Peer> Tracker::GetPeers(const Torrent& t, const std::string& peer_id, int listening_port) {
        Url url = Url::Parse(t.tracker_url);
        
        std::cout << "[Tracker] Contacting " << url.host << " via " << url.protocol << "..." << std::endl;

        if (url.protocol == "udp") {
            return GetPeersUDP(url, t, peer_id, listening_port);
        } else if (url.protocol == "http" || url.protocol == "https") {
            // Note: We don't support HTTPS (SSL) yet, treating as HTTP or fail
            return GetPeersHTTP(url, t, peer_id, listening_port);
        } else {
            throw std::runtime_error("Unknown protocol: " + url.protocol);
        }
    }

    std::vector<Peer> Tracker::GetPeersUDP(const Url& url, const Torrent& t, const std::string& peer_id, int port) {
        UdpClient udp(url.host, url.port);
        udp.SetTimeout(5); // 5 seconds timeout

        // --- Step 1: Connection Request ---
        Buffer connect_req(16);
        BufferUtils::WriteBE64(connect_req, 0, UDP_CONNECTION_MAGIC);
        BufferUtils::WriteBE32(connect_req, 8, ACTION_CONNECT);
        uint32_t trans_id = rand(); // Random ID to track this transaction
        BufferUtils::WriteBE32(connect_req, 12, trans_id);

        udp.Send(connect_req);
        Buffer connect_res = udp.Receive();

        if (connect_res.size() < 16) throw std::runtime_error("Invalid UDP connect response");
        uint64_t connection_id = BufferUtils::ReadBE64(connect_res, 8);

        // --- Step 2: Announce Request ---
        Buffer announce_req(98);
        BufferUtils::WriteBE64(announce_req, 0, connection_id);
        BufferUtils::WriteBE32(announce_req, 8, ACTION_ANNOUNCE);
        BufferUtils::WriteBE32(announce_req, 12, trans_id);
        
        std::copy(t.info_hash.begin(), t.info_hash.end(), announce_req.begin() + 16); // Info Hash
        std::copy(peer_id.begin(), peer_id.end(), announce_req.begin() + 36);         // Peer ID
        
        BufferUtils::WriteBE64(announce_req, 56, 0); // Downloaded
        BufferUtils::WriteBE64(announce_req, 64, t.length); // Left
        BufferUtils::WriteBE64(announce_req, 72, 0); // Uploaded
        BufferUtils::WriteBE32(announce_req, 80, 2); // Event: Started
        BufferUtils::WriteBE32(announce_req, 84, 0); // IP
        BufferUtils::WriteBE32(announce_req, 88, 0); // Key
        BufferUtils::WriteBE32(announce_req, 92, -1); // Num Want (-1 = default)
        BufferUtils::WriteBE16(announce_req, 96, port); // Port

        udp.Send(announce_req);
        Buffer response = udp.Receive();

        // --- Step 3: Parse Response ---
        if (response.size() < 20) throw std::runtime_error("Invalid UDP announce response");
        
        std::vector<Peer> peers;
        // Peers start at offset 20. Each peer is 6 bytes (4 IP + 2 Port)
        for (size_t i = 20; i + 6 <= response.size(); i += 6) {
            Peer p;
            p.ip = std::to_string(response[i]) + "." + 
                   std::to_string(response[i+1]) + "." + 
                   std::to_string(response[i+2]) + "." + 
                   std::to_string(response[i+3]);
            p.port = BufferUtils::ReadBE16(response, i + 4);
            peers.push_back(p);
        }
        return peers;
    }

    std::vector<Peer> Tracker::GetPeersHTTP(const Url& url, const Torrent& t, const std::string& peer_id, int port) {
        // Simplified HTTP implementation
        TcpClient tcp(url.host, url.port);
        Buffer req = HttpBuilder::BuildGetRequest(url, t.info_hash, peer_id, port);
        tcp.Send(req);
        
        std::cout << "[Tracker] HTTP Request sent. Waiting for response..." << std::endl;
        Buffer res = tcp.Receive(4096); 
        
        // In a real implementation, we would parse the Bencoded response here.
        // For this phase, we just acknowledge contact was made.
        std::cout << "[Tracker] HTTP Response received (" << res.size() << " bytes)." << std::endl;
        return {}; 
    }
}