#include "tracker/Tracker.h"
#include "tracker/Url.h"
#include "tracker/Transport.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <chrono>
#include <random>
#include <unistd.h>
#include <cstring>
#include <stdexcept>


namespace BitTorrent {

    std::string Tracker::UrlEncode(const Buffer& buffer) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (uint8_t c : buffer) {
            // Keep alphanumeric characters as is
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                // Encode others as %XX
                escaped << '%' << std::setw(2) << int(c);
            }
        }
        return escaped.str();
    }

    std::vector<Peer> Tracker::GetPeers(const TorrentFile& tf, const std::string& peer_id, int port) {
        Url url = Url::Parse(tf.announce);
        std::cout << "[Tracker] Contacting " << url.host << " (" << url.protocol << ")..." << std::endl;

        if (url.protocol == "http") {
            return GetPeersHTTP(url, tf, peer_id, port);
        } else if (url.protocol == "udp") {
            std::cout << "[Tracker] UDP not fully implemented in this phase. Trying anyway..." << std::endl;
             // Placeholder for UDP logic if we add it later
             return {}; 
        }
        throw std::runtime_error("Unsupported Tracker Protocol: " + url.protocol);
    }

    std::vector<Peer> Tracker::GetPeersHTTP(const Url& url, const TorrentFile& tf, const std::string& peer_id, int port) {
        TcpClient client(url.host, url.port);
        
        // 1. Build HTTP GET Request
        // GET /announce?info_hash=%86%F6...&peer_id=-BT1000-...&port=6881&compact=1 HTTP/1.1
        // Host: bttracker.debian.org
        // Key Detail: compact=1. This tells the server: "Don't send me a huge Dictionary. Send me a tiny binary blob of IPs."
        std::ostringstream req;
        req << "GET " << url.path 
            << "?info_hash=" << UrlEncode(tf.info_hash)
            << "&peer_id=" << peer_id
            << "&port=" << port
            << "&uploaded=0"
            << "&downloaded=0"
            << "&compact=1" // Important: Ask for binary peer list
            << "&left=" << tf.length 
            << " HTTP/1.1\r\n"
            << "Host: " << url.host << "\r\n"
            << "User-Agent: BitTorrent/1.0\r\n"
            << "Connection: Close\r\n\r\n";

        // 2. Send & Receive
        client.Send(BufferUtils::FromString(req.str()));
        Buffer response = client.Receive(8192); // Get up to 8KB

        // 3. Parse Response (Find "\r\n\r\n" to separate Header from Body)
        std::string res_str = BufferUtils::ToString(response);
        size_t header_end = res_str.find("\r\n\r\n");
        if (header_end == std::string::npos) return {};

        std::string body = res_str.substr(header_end + 4);

        // 4. Parse "Compact" Peer List
        // The tracker usually sends a Dictionary, but we can cheat and look for the binary string directly
        // because parsing the whole HTTP body as Bencode is complex if headers are attached.
        
        // Simpler approach: Find "5:peers" (or similar) and read the binary blob
        // For a robust client, we should strip headers and pass 'body' to Bnode::Decode.
        // Let's assume the body is valid Bencode.
        
        try {
            Buffer bodyBuf = BufferUtils::FromString(body);
            Bnode root = Bnode::Decode(bodyBuf); // Use our Parser!
            
            if (root.IsDict() && root.GetDict().count("peers")) {
                Bnode peersNode = root.GetDict().at("peers");
                
                // Case A: Binary String (Compact)
                if (peersNode.IsString()) {
                    std::string bin = peersNode.GetString();
                    std::vector<Peer> peers;
                    for (size_t i = 0; i + 6 <= bin.length(); i += 6) {
                        
                        // Extract IP
                        uint8_t p1 = bin[i];
                        uint8_t p2 = bin[i+1];
                        uint8_t p3 = bin[i+2];
                        uint8_t p4 = bin[i+3];
                        std::string ip = std::to_string(p1) + "." + std::to_string(p2) + "." + 
                                         std::to_string(p3) + "." + std::to_string(p4);
                        
                        // Extract Port (Big Endian)
                        uint16_t port = ((uint8_t)bin[i+4] << 8) | (uint8_t)bin[i+5];
                        peers.emplace_back(ip, port);
                    }
                    return peers;
                }
            }
        } catch (std::exception& e) {
            std::cerr << "[Tracker] Error parsing response: " << e.what() << std::endl;
        }

        return {};
    }

    // std::vector<Peer> Tracker::GetPeersUDP(const Url& url, const TorrentFile& tf, const std::string& peer_id, int port) {
    //     // We will implement UDP logic later if needed. 
    //     // HTTP is reliable enough for Debian.
    //     return {}; 
    // }
    // Local Helper: Random 32-bit Integer for Transaction ID
    uint32_t RandomTransactionID() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
        return dis(gen);
    }

    // Local Helper: Write Big Endian Integers
    void WriteBE32(Buffer& b, uint32_t val) {
        b.push_back((val >> 24) & 0xFF);
        b.push_back((val >> 16) & 0xFF);
        b.push_back((val >> 8) & 0xFF);
        b.push_back(val & 0xFF);
    }
    void WriteBE64(Buffer& b, uint64_t val) {
        WriteBE32(b, (val >> 32) & 0xFFFFFFFF);
        WriteBE32(b, val & 0xFFFFFFFF);
    }

    std::vector<Peer> Tracker::GetPeersUDP(const Url& url, const TorrentFile& tf, const std::string& peer_id, int port) {
        std::vector<Peer> result;
        int sock = -1;

        try {
            // 1. DNS Resolution (Hostname -> IP)
            struct addrinfo hints{}, *res;
            hints.ai_family = AF_INET;      // IPv4
            hints.ai_socktype = SOCK_DGRAM; // UDP

            if (getaddrinfo(url.host.c_str(), std::to_string(url.port).c_str(), &hints, &res) != 0) {
                throw std::runtime_error("DNS Resolution failed");
            }

            // 2. Create Socket
            sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if (sock < 0) {
                freeaddrinfo(res);
                throw std::runtime_error("Socket creation failed");
            }

            // Set Timeout (2 seconds) - UDP can get lost, we don't want to hang forever
            struct timeval tv;
            tv.tv_sec = 2; tv.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            // --- STEP 1: CONNECT REQUEST ---
            // Format: [ProtocolID (8 bytes)] [Action=0 (4 bytes)] [TransID (4 bytes)]
            Buffer connect_req;
            uint64_t magic_protocol_id = 0x41727101980; 
            uint32_t action_connect = 0;
            uint32_t transaction_id = RandomTransactionID();

            WriteBE64(connect_req, magic_protocol_id);
            WriteBE32(connect_req, action_connect);
            WriteBE32(connect_req, transaction_id);

            if (sendto(sock, connect_req.data(), connect_req.size(), 0, res->ai_addr, res->ai_addrlen) < 0) {
                throw std::runtime_error("Send Connect failed");
            }

            // Receive Connect Response
            Buffer recv_buf(1024);
            ssize_t len = recvfrom(sock, recv_buf.data(), 1024, 0, nullptr, nullptr);
            if (len < 16) throw std::runtime_error("Connect response too short or timed out");

            // Verify: [Action (4)] [TransID (4)] [ConnectionID (8)]
            // We reuse BufferUtils or manual shifting logic
            uint32_t recv_action = (recv_buf[0]<<24)|(recv_buf[1]<<16)|(recv_buf[2]<<8)|recv_buf[3];
            uint32_t recv_trans_id = (recv_buf[4]<<24)|(recv_buf[5]<<16)|(recv_buf[6]<<8)|recv_buf[7];
            
            if (recv_action != 0 || recv_trans_id != transaction_id) {
                throw std::runtime_error("Invalid Connect Response");
            }

            uint64_t connection_id = 0;
            for(int i=0; i<8; i++) connection_id = (connection_id << 8) | recv_buf[8+i];

            // --- STEP 2: ANNOUNCE REQUEST ---
            // Format: [ConnID (8)] [Action=1 (4)] [TransID (4)] [InfoHash (20)] [PeerID (20)] [Downloaded (8)] ...
            Buffer ann_req;
            WriteBE64(ann_req, connection_id);      // Connection ID from Step 1
            WriteBE32(ann_req, 1);                  // Action = 1 (Announce)
            WriteBE32(ann_req, transaction_id);     // Trans ID (Reuse or new)
            
            // Info Hash & Peer ID
            ann_req.insert(ann_req.end(), tf.info_hash.begin(), tf.info_hash.end());
            ann_req.insert(ann_req.end(), peer_id.begin(), peer_id.end());

            WriteBE64(ann_req, 0); // Downloaded
            WriteBE64(ann_req, tf.length); // Left
            WriteBE64(ann_req, 0); // Uploaded
            WriteBE32(ann_req, 0); // Event (0 = None)
            WriteBE32(ann_req, 0); // IP (0 = Default)
            WriteBE32(ann_req, RandomTransactionID()); // Key (Random)
            WriteBE32(ann_req, -1); // Num_Want (-1 = Default)
            
            // Port (2 bytes)
            ann_req.push_back((port >> 8) & 0xFF);
            ann_req.push_back(port & 0xFF);

            if (sendto(sock, ann_req.data(), ann_req.size(), 0, res->ai_addr, res->ai_addrlen) < 0) {
                throw std::runtime_error("Send Announce failed");
            }

            // Receive Announce Response
            len = recvfrom(sock, recv_buf.data(), 1024, 0, nullptr, nullptr);
            if (len < 20) throw std::runtime_error("Announce response too short");

            // Response Format: [Action (4)] [TransID (4)] [Interval (4)] [Leechers (4)] [Seeders (4)] [Peers...]
            // Peers start at offset 20. Each peer is 6 bytes (IP: 4, Port: 2)
            
            for (size_t i = 20; i + 6 <= len; i += 6) {
                Peer p;
                // IP (Bytes i to i+3)
                p.ip = std::to_string((int)recv_buf[i]) + "." + 
                       std::to_string((int)recv_buf[i+1]) + "." + 
                       std::to_string((int)recv_buf[i+2]) + "." + 
                       std::to_string((int)recv_buf[i+3]);
                
                // Port (Bytes i+4, i+5)
                p.port = (recv_buf[i+4] << 8) | recv_buf[i+5];
                
                result.push_back(p);
            }
            
            std::cout << "[Tracker UDP] Found " << result.size() << " peers." << std::endl;
            freeaddrinfo(res);
            close(sock);

        } catch (const std::exception& e) {
            std::cerr << "[Tracker UDP Error] " << e.what() << std::endl;
            if (sock >= 0) close(sock);
        }

        return result;
    }
}