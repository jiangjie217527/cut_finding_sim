#ifndef HGS_SCHEDULER_HPP_
#define HGS_SCHEDULER_HPP_

#include "common.hpp"
#include "DCache.hpp"
#include "DRAM.hpp"
#include "types.hpp"

#include <queue>
#include <tuple>
#include <unordered_set>

struct Scheduler {
    int cycle = 0;
    std::queue<int> task_queue;
    std::queue<std::tuple<int, int, int, bool>> leaf_to_submit;
    std::queue<std::pair<int, int>> tasks_to_submit;
    std::unordered_set<int> if_leaves_submitted;
    std::queue<int> tasks_waitlist;
    std::vector<int> tasks_loaded_to_cache;

    bool schedule(DCache &dcache, DRAM &dram);

    void recycle();

    void print_len();

    bool isBusy();
};

#endif // HGS_SCHEDULER_HPP_
