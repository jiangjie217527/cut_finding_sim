#include "scheduler.hpp"

bool Scheduler::schedule(DCache &dcache) {
    bool result = false;

    cycle++;
    if (leaf_to_submit.empty() && tasks_to_submit.empty() && task_queue.empty()) {
        return false;
    }

    if (!leaf_to_submit.empty()) {
        auto leaf = leaf_to_submit.front();
        int time_stamp = std::get<0>(leaf);
        int leaf_id = std::get<1>(leaf);
        bool is_end = std::get<2>(leaf);
        if (time_stamp <= cycle) {
            
        }
    }
}