#ifndef BENCODE_H
#define BENCODE_H

#include <vector>
#include <stdexcept>
#include <map>
#include "parsing/buffer.h"
#include <string>
#include <any>
using namespace std;
class bencode
{
public:
    enum type {bs,i,l,d};
    struct item
    {
        any data;
        type t;

        bool operator==(const item &other) const;
        bool operator<(const item &other) const;
        buffer get_buffer(const char *key) const;
        long long get_int(const char *key) const;
        string get_string(const char *key) const;
        item get_item(const char *key) const;
        vector<bencode::item> get_list(const char *key) const;
        bool key_present(const char *key) const;

    private:
        item get(const item &key) const;
    };
    private:
        typedef buffer::size_type size;
        static void next(bencode::item& e,const buffer& data,size& offset);
        static item parse_byte_string(buffer const& data,size& offset);
        static item parse_integer(buffer const& data,size& offset);
        static item parse_list(buffer const& data,size& offset);
        static item parse_dictionary(buffer const& data,size& offset);
    public:
        bencode()=delete;
        struct invalid_bencode: invalid_argument{
            invalid_bencode(): invalid_argument("") {}
            invalid_bencode(const char* message ): invalid_argument(message ){}
        };
        static item parse(buffer const& data);
        static buffer encode(item const& data);
        static void print(const item& element);
}

#endif