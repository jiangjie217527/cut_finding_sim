#ifndef HGS_SCHEDULER_HPP_
#define HGS_SCHEDULER_HPP_

#include "common.hpp"
#include "types.hpp"

#include <queue>

struct Scheduler {
    std::queue<int> task_queue;

    void schedule();  
};

#endif // HGS_SCHEDULER_HPP_