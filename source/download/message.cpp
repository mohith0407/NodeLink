#include "download/message.h"
#include "download/peer_id.h"

using namespace std;

buffer message::build_handshake(const torrent& t){}

buffer message::build_keep_alive() return buffer(4);

buffer message::build_choke(){
	buffer b(5);
	b[3]=1;
	b[4]=0;
	return b;
}

buffer message::build_unchoke(){
	buffer b(5);
	b[3]=1;
	b[4]=1;
	return b;
}

buffer message::build_interested(){
	buffer b(5);
	b[3]=1;
	b[4]=2;
	return b;
}

buffer message::build_not_interested(){
	buffer b(5);
	b[3]=1;
	b[4]=3;
	return b;
}

buffer message::build_have(const buffer& payload){
	buffer b(9);
	b[3]=5;
	b[4]=4;
	copy(payload.begin(), payload.end(), b.begin()+5);
	return b;
}

buffer message::build_bitfield(const buffer& bitfield) {}

buffer message::build_request(unsigned int index,unsigned int begin,unsigned int length){}

buffer message::build_piece(unsigned int index,unsigned int begin,const buffer& block){} 

buffer message::build_cancel(unsigned int index,unsigned int begin,unsigned int length){}

buffer message::build_port(unsigned int port){}