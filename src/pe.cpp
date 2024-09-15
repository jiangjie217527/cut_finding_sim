#include "pe.hpp"

InnerTask::InnerTask(int offset, int inner_id) : offset(offset), inner_id(inner_id) {}

PE::PE(std::vector<int>& render_indices,
                  std::vector<int>& parent_indices) : render_indices(render_indices), parent_indices(parent_indices) {
    for (int i = 0; i < PipelineStage; ++i) {
        inner_tasks[i] = InnerTask(i, i);
    }
}

bool PE::updateTick(std::queue<int> &task_queue, DCache &dcache) {
    bool res = false;
    for (int i = 0; i < PipelineStage; ++i) {
        res |= inner_tasks[i].updateTick(task_queue, dcache);
    }

    return res;
}

bool InnerTask::updateTick(std::queue<int> &task_queue, DCache &dcache) {
    this->cycle++;
    
    if (!busy) {
        if (cycle % PipelineStage != offset) {
            return false;
        }

        if (!task_queue.empty()) {
            this->cur_id = task_queue.front();
            Task cur_task;
            std::vector<Node> nodes;
            std::vector<Box> boxes;

            if (!dcache.readData(cur_id, cur_task, nodes, boxes)) {
                return false; // bank conflict
            }

            this->cur_task = cur_task;
            this->inner_id = cur_id;
            this->busy = true;
            this->cur_id = cur_task.start_id;

            
            while (cur_id <= cur_task.start_id + cur_task.task_size && cur_id >= cur_task.start_id) {
                // todo judgement process
            }
        }
    } else {

    }
}