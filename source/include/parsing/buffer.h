#ifndef BUFFER_H
#define BUFFER_H
#include<vector>
typedef std::vector<unsigned char> buffer;
// data b,offset idx
unsigned int getBE16(const buffer& data,buffer::size_type offset);
unsigned int getBE32(const buffer& data,buffer::size_type ofset);
// value n
buffer setBE16(unsigned int value,buffer& data,const buffer::size_type offset);
buffer setBE32(unsigned int value,buffer& data,const buffer::size_type offset);
void print(const buffer& data);

#endif