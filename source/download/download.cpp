#include "download/download.h"
#include "download/connection.h"
#include "download/farm.h"
#include <thread>
#include <algorithm>
#include <cassert>
#include <stdlib.h>
#include <iostream>

using namespace std;

download::download(const vector<peer>& peers,torrent& t): t(t), peers(peers), received_count(0), w(t.name){}

void download::start(){}

void download::add_received(int piece,int block,buffer place_data){}

bool download::is_done(){}
double download::completed(){}

void download::push_job(job j){}

download::job download::pop_job(){}