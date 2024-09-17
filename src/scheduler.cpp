#include "scheduler.hpp"
#include "pe.hpp"

bool Scheduler::schedule(DCache &dcache) {
    bool result = false;

    cycle++;
    if (leaf_to_submit.empty() && tasks_to_submit.empty() && task_queue.empty()) {
        return false;
    }

    if (!leaf_to_submit.empty()) {
        result = true;
        auto leaf = leaf_to_submit.front();
        int task_id = std::get<0>(leaf);
        int time_stamp = std::get<1>(leaf);
        int leaf_id = std::get<2>(leaf);
        bool is_end = std::get<3>(leaf);
        if (time_stamp <= cycle) {
            // find the corresponding sub_tasks in the cache
            std::vector<int> leaves, leaf_task_ids;
            if (dcache.readSubtask(task_id, leaves, leaf_task_ids)) {
                for (int i = 0; i < leaves.size(); ++i) {
                    if (leaves[i] == leaf_id) {
                        tasks_waitlist.push(leaf_task_ids[i]);
                    }

                    if (is_end) {
                        if_leaves_submitted.insert(leaf_id);
                    }
                }
            }
        }

        if (!tasks_to_submit.empty()) {
            int time_stamp = tasks_to_submit.front().first;
            int task_id = tasks_to_submit.front().second;

            if (time_stamp <= cycle) {
                
            }
        }
    }
}
