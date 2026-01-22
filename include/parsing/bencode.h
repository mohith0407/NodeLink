#ifndef BENCODE_H
#define BENCODE_H

#include "parsing/buffer.h"
#include <vector>
#include <map>
#include <any>
#include <string>
#include <stdexcept>

namespace BitTorrent {

    class Bencode {
    public:
        // The 4 types supported by Bencode
        enum Type { STRING, INT, LIST, DICT };

        struct Item {
            std::any data; // Holds the actual value (int, string, etc.)
            Type type;     // Tells us what type is inside 'data'

            // Required to use Item as a key in std::map
            bool operator<(const Item& other) const;
            bool operator==(const Item& other) const;

            // --- Accessor Methods (Getters) ---
            // These extract the value from 'std::any' safely
            Buffer      asBuffer() const;
            std::string asString() const;
            long long   asInt() const;
            std::vector<Item> asList() const;
            
            // --- Dictionary Helpers ---
            // "Give me the value for key 'announce'"
            Item        getFromDict(const std::string& key) const;
            bool        hasKey(const std::string& key) const;
            
            // Shortcuts: "Give me the 'announce' value directly as a string"
            Buffer      getBufferFromDict(const std::string& key) const;
            long long   getIntFromDict(const std::string& key) const;
            std::string getStringFromDict(const std::string& key) const;
            std::vector<Item> getListFromDict(const std::string& key) const;
        };

        static Item Parse(const Buffer& data);
        
        // We need Encode to calculate the SHA1 hash of the 'info' dictionary
        static Buffer Encode(const Item& item);

    private:
        // Recursive parsing functions
        static void decodeNext(Item& item, const Buffer& data, size_t& idx);
        static Item decodeString(const Buffer& data, size_t& idx);
        static Item decodeInt(const Buffer& data, size_t& idx);
        static Item decodeList(const Buffer& data, size_t& idx);
        static Item decodeDict(const Buffer& data, size_t& idx);
    };
}
#endif