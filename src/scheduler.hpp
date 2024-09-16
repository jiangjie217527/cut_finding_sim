#ifndef HGS_SCHEDULER_HPP_
#define HGS_SCHEDULER_HPP_

#include "common.hpp"
#include "DCache.hpp"
#include "types.hpp"

#include <queue>
#include <tuple>

struct Scheduler {
    int cycle = 0;
    std::queue<int> task_queue;
    std::queue<std::tuple<int, int, bool>> leaf_to_submit;
    std::queue<std::pair<int, int>> tasks_to_submit;

    bool schedule(DCache &dcache);
};

#endif // HGS_SCHEDULER_HPP_