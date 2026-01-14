#include "download/worker.h"
// for time and dates
#include <chrono>
#include <vector>
#include <iostream>

using namespace std;

void writer::start(){}

void writer::stop(){}

void writer::add(buffer& b,int offset){}

void speed::start(){}

void speed::stop(){}

void speed::add(unsigned int b){}

void speed::draw(double progress,unsigned int speed){}

void speed::set_total(long long t) {total=t;}
void speed::human_readable(unsigned int b){}