#include "download/farm.h"
#include <stdexcept>
#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace std;

farm::farm(vector<connection>& conns, download& d): conns(conns), d(d){}

void farm::hatch(){}