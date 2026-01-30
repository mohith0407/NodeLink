#include "parsing/Bnode.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace BitTorrent {

    Bnode Bnode::Decode(const Buffer& data) {
        size_t index = 0;
        return DecodeElement(data, index);
    }

    Bnode Bnode::DecodeElement(const Buffer& data, size_t& index) {
        if (index >= data.size()) throw std::runtime_error("Unexpected end of buffer");

        char c = (char)data[index];

        // 1. Integer: starts with 'i', ends with 'e'
        if (c == 'i') {
            index++; // skip 'i'
            size_t end = index;
            while (end < data.size() && data[end] != 'e') end++;
            if (end >= data.size()) throw std::runtime_error("Invalid Integer");

            std::string numStr(data.begin() + index, data.begin() + end);
            index = end + 1; // skip 'e'
            return Bnode(std::stoll(numStr));
        }

        // 2. List: starts with 'l', ends with 'e'
        else if (c == 'l') {
            index++; // skip 'l'
            BList list;
            while (index < data.size() && data[index] != 'e') {
                list.push_back(DecodeElement(data, index));
            }
            if (index >= data.size()) throw std::runtime_error("Unclosed List");
            index++; // skip 'e'
            return Bnode(list);
        }

        // 3. Dictionary: starts with 'd', ends with 'e'
        else if (c == 'd') {
            index++; // skip 'd'
            BDict dict;
            while (index < data.size() && data[index] != 'e') {
                // Keys must be strings
                Bnode key = DecodeElement(data, index);
                if (!key.IsString()) throw std::runtime_error("Dict key must be string");
                
                Bnode val = DecodeElement(data, index);
                dict[key.GetString()] = val;
            }
            if (index >= data.size()) throw std::runtime_error("Unclosed Dictionary");
            index++; // skip 'e'
            return Bnode(dict);
        }

        // 4. String: starts with "length:", e.g. "4:spam"
        else if (isdigit(c)) {
            size_t colon = index;
            while (colon < data.size() && data[colon] != ':') colon++;
            if (colon >= data.size()) throw std::runtime_error("Invalid String length");

            std::string lenStr(data.begin() + index, data.begin() + colon);
            long long len = std::stoll(lenStr);
            
            index = colon + 1; // move past ':'
            if (index + len > data.size()) throw std::runtime_error("String out of bounds");

            std::string s(data.begin() + index, data.begin() + index + len);
            index += len;
            return Bnode(s);
        }

        throw std::runtime_error("Unknown Bencode type");
    }

    // Encoding (Needed for hashing the 'info' dictionary)
    Buffer Bnode::Encode(const Bnode& node) {
        Buffer out;
        if (node.IsInt()) {
            std::string s = "i" + std::to_string(node.GetInt()) + "e";
            out.insert(out.end(), s.begin(), s.end());
        }
        else if (node.IsString()) {
            std::string val = node.GetString();
            std::string s = std::to_string(val.length()) + ":" + val;
            out.insert(out.end(), s.begin(), s.end());
        }
        else if (node.IsList()) {
            out.push_back('l');
            for (const auto& item : node.GetList()) {
                Buffer sub = Encode(item);
                out.insert(out.end(), sub.begin(), sub.end());
            }
            out.push_back('e');
        }
        else if (node.IsDict()) {
            out.push_back('d');
            for (const auto& pair : node.GetDict()) {
                // Encode key (string)
                std::string kLen = std::to_string(pair.first.length()) + ":" + pair.first;
                out.insert(out.end(), kLen.begin(), kLen.end());
                
                // Encode Value
                Buffer sub = Encode(pair.second);
                out.insert(out.end(), sub.begin(), sub.end());
            }
            out.push_back('e');
        }
        return out;
    }
}