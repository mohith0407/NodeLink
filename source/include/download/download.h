#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <vector>
#include "tracker/tracker.h"
#include <queue>
#include "download/worker.h"
using namespace std;
// static primarily manages storage duration and linkage (lifetime and visibility)
class download{
    private:
        vector<vector<bool>> state_received, state_queued;
        const vector<peer>& remote_peers;
        torrent& meta;writer disc_writer;spped rate;
        int count_received,count_total_blocks;
    public:
        download(const vector<peer>& peer_list, torrent& torrent_ref);
        void add_received(int piece_index, int block_offset, buffer payload);
        void start();
        double completed();
        bool is_done();

        struct job {
            int index, begin, length, priority_level;
            job(int idx, int start_offset, int len) : index(idx), begin(start_offset), length(len), priority_level(0) {}
            job() {}
            bool operator<(const job& other) const { return this->priority_level > other.priority_level; }
        };

        void push_job(job task);
        job pop_job();
        static const int BLOCK_SIZE = (1 << 14);

    private:
        priority_queue<job> pending_tasks;
};


#endif