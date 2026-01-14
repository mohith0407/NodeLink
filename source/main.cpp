#include "parsing/torrent.h"
#include "download/peer_id.h"
#include "tracker/tracker.h"
#include "download/download.h"
#include <iostream>
using namespace std;

int main(int argc,const char** argv){
    if(argc<2){
        cout<<"usage: BitTorrent <torrent_file>"<<endl;
        return 0;
    }
    srand(time(null));
    peer_id::generate();
    torrent t(argv[1]);
    cout<<"Fetching peers from tracker...(Hang tight it's loading)"<<endl;
    vector<peer> v=tracker::get_peers(t);
    cout<<"Received "<<v.size()<<" peers"<<endl;
    download d(v,t);
    d.start();
    cout<<"Successfully downloaded"<<endl;
    return 0;
}