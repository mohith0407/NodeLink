#include "tracker/tracker.h"
#include "tracker/tcp.h"
#include "parsing/buffer.h"
#include "download/peer_id.h"
#include "download/connection.h"
#include "download/message.h"
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <unistd.h>

using namespace std;

// we can use this
// #define connection connection::
connection::connection(const peer& p,torrent& t,download& d):p(p), d(d), t(t), buff(buffer()), handshake(true), choked(true), connected(false), socket(p.host,p.port,false){}

void connection::handle(buffer& mag){}

void connection::choke_handler(){}

void connection::unchoke_handler(){}
void connection::have_handler(buffer& b){}
void connection::bitfield_handler(buffer& b){}
void connection::request_piece(){}
void connection::piece_handler(buffer& b){}
void connection::ready(){}
void connection::enqueue(int piece){}

// #undef