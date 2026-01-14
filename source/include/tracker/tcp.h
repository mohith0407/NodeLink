#ifndef TCP_H
#define TCP_H

#include "tracker/transport.h"
#include <sys/types.h>

class tcp: public trasport {
    public:
        tcp(std:: string address,int port,bool blocking=true);
        trasport(address,port,SOCK_STREAM,blocking){}
};
#endif