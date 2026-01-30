#include "parsing/TorrentFile.h"
#include <fstream>
#include <iostream>
#include <openssl/sha.h> // Requires -lcrypto

namespace BitTorrent {

    TorrentFile TorrentFile::Load(const std::string& filepath) {
        // 1. Read File
        std::ifstream file(filepath, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open file: " + filepath);
        // Explanation: This is a C++ "one-liner" to read a file.
        // iterator<char>(file) points to the Start of the file.
        // iterator<char>() (empty) points to the End of the file.
        // The Buffer constructor says: "Copy everything from Start to End."
        Buffer data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        // 2. Decode Bencode
        Bnode root = Bnode::Decode(data);
        const auto& rootDict = root.GetDict();

        TorrentFile t;
        t.announce = rootDict.at("announce").GetString();

        // 3. Process 'info' dictionary
        // We MUST keep 'info' as a Bnode to hash it later
        if (rootDict.count("info") == 0) throw std::runtime_error("Invalid Torrent: Missing info");
        const Bnode& infoNode = rootDict.at("info");
        const auto& infoDict = infoNode.GetDict();

        t.name = infoDict.at("name").GetString();
        t.piece_length = infoDict.at("piece length").GetInt();
        
        // Handle Length (Single file mode for now)
        if (infoDict.count("length")) {
            t.length = infoDict.at("length").GetInt();
        } else {
             throw std::runtime_error("Multi-file torrents not supported yet");
        }

        // 4. Extract Pieces (Split big string into 20-byte chunks)
        std::string piecesBlob = infoDict.at("pieces").GetString();
        if (piecesBlob.length() % 20 != 0) throw std::runtime_error("Invalid pieces length");

        for (size_t i = 0; i < piecesBlob.length(); i += 20) {
            t.piece_hashes.push_back(piecesBlob.substr(i, 20));
        }

        // 5. Calculate Info Hash (CRITICAL STEP)
        // We re-encode the 'info' node back to raw bytes
        Buffer infoBytes = Bnode::Encode(infoNode);
        
        t.info_hash.resize(20);
        SHA1(infoBytes.data(), infoBytes.size(), t.info_hash.data());

        return t;
    }
}