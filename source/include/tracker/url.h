#ifndef URL_H
#define URL_H
#include<string>
using namespace std;
struct url_t{
    string host;
    string path;
    int port;
    enum {UDP,HTTP} protocol;
    url_t(const string& url);
    url_t() {};
};
#endif