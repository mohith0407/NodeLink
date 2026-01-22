#include "parsing/bencode.h"
#include <iostream>
#include <algorithm>

namespace BitTorrent {

    // --- Comparators (Required for std::map keys) ---
    bool Bencode::Item::operator<(const Item& other) const {
        if (type != other.type) return type < other.type;
        if (type == INT) 
            return std::any_cast<long long>(data) < std::any_cast<long long>(other.data);
        if (type == STRING) {
            const Buffer& a = std::any_cast<const Buffer&>(data);
            const Buffer& b = std::any_cast<const Buffer&>(other.data);
            return a < b;
        }
        return false; // Lists/Dicts not supported as keys
    }

    bool Bencode::Item::operator==(const Item& other) const {
        if (type != other.type) return false;
        if (type == INT) 
            return std::any_cast<long long>(data) == std::any_cast<long long>(other.data);
        if (type == STRING) {
             const Buffer& a = std::any_cast<const Buffer&>(data);
             const Buffer& b = std::any_cast<const Buffer&>(other.data);
             return a == b;
        }
        return false;
    }

    // --- Parsing Logic ---

    Bencode::Item Bencode::decodeString(const Buffer& s, size_t& idx) {
        size_t len = 0;
        while (idx < s.size() && isdigit(s[idx])) {
            len = len * 10 + (s[idx] - '0');
            idx++;
        }
        if (idx >= s.size() || s[idx] != ':') throw std::runtime_error("Invalid string format");
        idx++; // skip ':'
        
        Buffer result;
        if (idx + len <= s.size()) {
            result.assign(s.begin() + idx, s.begin() + idx + len);
        } else { throw std::runtime_error("String length out of bounds"); }
        
        idx += len;
        return {result, STRING};
    }

    Bencode::Item Bencode::decodeInt(const Buffer& s, size_t& idx) {
        idx++; // skip 'i'
        int sign = 1;
        if (idx < s.size() && s[idx] == '-') { sign = -1; idx++; }

        long long res = 0;
        while (idx < s.size() && isdigit(s[idx])) {
            res = res * 10 + (s[idx] - '0');
            idx++;
        }
        if (idx >= s.size() || s[idx] != 'e') throw std::runtime_error("Invalid integer format");
        idx++; // skip 'e'
        return {res * sign, INT};
    }

    Bencode::Item Bencode::decodeList(const Buffer& s, size_t& idx) {
        idx++; // skip 'l'
        std::vector<Item> list;
        while (idx < s.size() && s[idx] != 'e') {
            Item e;
            decodeNext(e, s, idx);
            list.push_back(e);
        }
        if (idx >= s.size()) throw std::runtime_error("List not terminated");
        idx++; // skip 'e'
        return {list, LIST};
    }

    Bencode::Item Bencode::decodeDict(const Buffer& s, size_t& idx) {
        idx++; // skip 'd'
        std::map<Item, Item> dict;
        while (idx < s.size() && s[idx] != 'e') {
            if (!isdigit(s[idx])) throw std::runtime_error("Dict key must be string");
            Item key = decodeString(s, idx);
            Item val;
            decodeNext(val, s, idx);
            dict[key] = val;
        }
        if (idx >= s.size()) throw std::runtime_error("Dict not terminated");
        idx++; // skip 'e'
        return {dict, DICT};
    }

    void Bencode::decodeNext(Item& e, const Buffer& s, size_t& idx) {
        if (idx >= s.size()) throw std::runtime_error("Unexpected end of buffer");
        char c = s[idx];
        if (c == 'i')       e = decodeInt(s, idx);
        else if (c == 'l')  e = decodeList(s, idx);
        else if (c == 'd')  e = decodeDict(s, idx);
        else if (isdigit(c)) e = decodeString(s, idx);
        else throw std::runtime_error("Unknown type");
    }

    Bencode::Item Bencode::Parse(const Buffer& s) {
        size_t idx = 0;
        Item e;
        decodeNext(e, s, idx);
        return e;
    }

    // --- Encoding (Needed for SHA1 hash) ---
    Buffer Bencode::Encode(const Item& item) {
        Buffer ans;
        auto appendStr = [&](const std::string& s) {
            ans.insert(ans.end(), s.begin(), s.end());
        };

        if (item.type == INT) {
            appendStr("i" + std::to_string(std::any_cast<long long>(item.data)) + "e");
        } else if (item.type == STRING) {
            Buffer b = std::any_cast<Buffer>(item.data);
            appendStr(std::to_string(b.size()) + ":");
            ans.insert(ans.end(), b.begin(), b.end());
        } else if (item.type == LIST) {
            appendStr("l");
            for (const auto& i : std::any_cast<const std::vector<Item>&>(item.data)) {
                Buffer sub = Encode(i);
                ans.insert(ans.end(), sub.begin(), sub.end());
            }
            appendStr("e");
        } else if (item.type == DICT) {
            appendStr("d");
            for (const auto& [k, v] : std::any_cast<const std::map<Item, Item>&>(item.data)) {
                Buffer kb = Encode(k);
                Buffer vb = Encode(v);
                ans.insert(ans.end(), kb.begin(), kb.end());
                ans.insert(ans.end(), vb.begin(), vb.end());
            }
            appendStr("e");
        }
        return ans;
    }

    // --- Accessors Implementation ---

    Buffer Bencode::Item::asBuffer() const {
        return std::any_cast<Buffer>(data);
    }
    std::string Bencode::Item::asString() const {
        Buffer b = asBuffer();
        return std::string(b.begin(), b.end());
    }
    long long Bencode::Item::asInt() const {
        return std::any_cast<long long>(data);
    }
    std::vector<Bencode::Item> Bencode::Item::asList() const {
        return std::any_cast<std::vector<Item>>(data);
    }

    Bencode::Item Bencode::Item::getFromDict(const std::string& key) const {
        if (type != DICT) throw std::runtime_error("Not a dictionary");
        
        Item keyItem; 
        keyItem.type = STRING;
        Buffer keyBuf(key.begin(), key.end());
        keyItem.data = keyBuf;

        const auto& map = std::any_cast<const std::map<Item, Item>&>(data);
        auto it = map.find(keyItem);
        if (it == map.end()) throw std::runtime_error("Key not found: " + key);
        return it->second;
    }

    bool Bencode::Item::hasKey(const std::string& key) const {
        if (type != DICT) return false;
        Item keyItem; keyItem.type = STRING;
        Buffer keyBuf(key.begin(), key.end());
        keyItem.data = keyBuf;
        
        try {
            const auto& map = std::any_cast<const std::map<Item, Item>&>(data);
            return map.find(keyItem) != map.end();
        } catch (...) { return false; }
    }

    // Helpers
    Buffer Bencode::Item::getBufferFromDict(const std::string& key) const { return getFromDict(key).asBuffer(); }
    long long Bencode::Item::getIntFromDict(const std::string& key) const { return getFromDict(key).asInt(); }
    std::string Bencode::Item::getStringFromDict(const std::string& key) const { return getFromDict(key).asString(); }
    std::vector<Bencode::Item> Bencode::Item::getListFromDict(const std::string& key) const { return getFromDict(key).asList(); }
}