#include "tracker/url.h"
#include <stdexcept>
#include <algorithm>

using namespace std;

url_t::url_t(const string& url){
    string input=url;
    string prefixUdp="udp://", prefixHttp="http://";

    if(input.substr(0,prefixUdp.size())==prefixUdp){
        s.erase(0,prefixUdp.size());
        this->protocol=UDP;
    }
    else if(input.substr(0,prefixHttp.size()) == prefixHttp) {
		input.erase(0,prefixHttp.size());
		this->protocol = HTTP;

	}
    else{ throw runtime_error("Undefined protocol in url");}

    auto it=find(input.begin(),input.end(),':');
    string host;
    copy(input.begin(),it,back_inserter(host));
    it++;
    input.erase(input.begin(),it);
    this->host=move(host);
    this->port=stoi(input);

    it = find(input.begin(), input.end(), '/');
	input.erase(input.begin(), it);
	this->path = move(input);

}