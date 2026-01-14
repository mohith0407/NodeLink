#include "download/peer_id.h"
#include <cstdlib>
#include <algorithm>

using namespace std;

buffer peer_id::id;
buffer peer_id::get() {return id;}

void peer_id::generate(const string& pref){

}