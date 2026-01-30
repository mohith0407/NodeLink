#pragma once

#include "Buffer.h"
#include <variant>
#include <map>
#include <memory>

namespace BitTorrent {

    // Forward declaration because a List contains Bnodes
    class Bnode;

    // Define the possible types a Bnode can hold
    using BList = std::vector<Bnode>;
    using BDict = std::map<std::string, Bnode>;
    using BInt  = int64_t;
    using BString = std::string; // We treat byte strings as std::string for simplicity

    class Bnode {
    public:
        // The actual data is stored here
        std::variant<BInt, BString, BList, BDict> value;

        // Constructors
        Bnode() = default;
        Bnode(BInt v) : value(v) {}
        Bnode(BString v) : value(v) {}
        Bnode(BList v) : value(v) {}
        Bnode(BDict v) : value(v) {}

        // Helper methods to check type
        bool IsInt() const { return std::holds_alternative<BInt>(value); }
        bool IsString() const { return std::holds_alternative<BString>(value); }
        bool IsList() const { return std::holds_alternative<BList>(value); }
        bool IsDict() const { return std::holds_alternative<BDict>(value); }

        // Getters (Safe Access)
        BInt GetInt() const { 
            if(!IsInt()) throw std::runtime_error("Type mismatch: Expected Int");
            return std::get<BInt>(value); 
        }
        
        const BString& GetString() const { 
            if(!IsString()) throw std::runtime_error("Type mismatch: Expected String");
            return std::get<BString>(value); 
        }
        
        const BList& GetList() const { 
            if(!IsList()) throw std::runtime_error("Type mismatch: Expected List");
            return std::get<BList>(value); 
        }

        const BDict& GetDict() const { 
            if(!IsDict()) throw std::runtime_error("Type mismatch: Expected Dict");
            return std::get<BDict>(value); 
        }

        // Static Decoding Function
        static Bnode Decode(const Buffer& data);
        static Buffer Encode(const Bnode& node);

    private:
        static Bnode DecodeElement(const Buffer& data, size_t& index);
    };
}