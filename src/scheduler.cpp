#include "scheduler.hpp"

bool Scheduler::schedule(DCache &dcache, DRAM &dram) {
  bool result = false;

  cycle++;

  dcache.loadBufferCache();

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
        }

        if (is_end) {
          if_leaves_submitted.insert(task_id);
        }

        leaf_to_submit.pop();
      }
    }
  }

  if (!tasks_to_submit.empty()) {
    result = true;
    int time_stamp = tasks_to_submit.front().first;
    int task_id = tasks_to_submit.front().second;

    if (time_stamp <= cycle && if_leaves_submitted.find(task_id) != if_leaves_submitted.end()) {
      // can kick the task out of DCache
      dcache.invalidate(task_id);
      tasks_to_submit.pop();
    }
  }

  if (!tasks_waitlist.empty()) {
    result = true;
    int task_id = tasks_waitlist.front();
    if (dcache.cacheLoadData(task_id, dram)) {
      tasks_waitlist.pop();
    } else if (dcache.bufferCacheLoadData(task_id, dram)) {
      tasks_waitlist.pop();
    }
  }


  return result;
}
