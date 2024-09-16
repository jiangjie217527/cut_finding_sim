#include "pe.hpp"
#include "common.hpp"
#include "utils.hpp"

InnerTask::InnerTask(int offset, int inner_id, PE* parent_pe) : offset(offset), inner_id(inner_id) {
    this->parent_pe = parent_pe;
}

PE::PE(std::vector<int>& render_indices,
                  std::vector<int>& parent_indices) : render_indices(render_indices), parent_indices(parent_indices) {
    for (int i = 0; i < PipelineStage; ++i) {
        inner_tasks[i] = InnerTask(i, i, this);
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

        if (task_queue.empty() && this->inner_id == -1) {
            return false;
        }

        if (this->inner_id == -1) {
            this->cur_id = task_queue.front();
            task_queue.pop();
        }

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

        int dealt_points = 0;
            
        while (cur_id <= cur_task.start_id + cur_task.task_size && cur_id >= cur_task.start_id) {
            // todo judgement process
            dealt_points++;

            int id = cur_id - cur_task.start_id;
            Point viewpoint = {this->parent_pe->viewpoint[0], this->parent_pe->viewpoint[1], this->parent_pe->viewpoint[2]};
            float size = computeSize(boxes[id], viewpoint);
            bool selected = false, in_fr = in_frustum(boxes[id], this->parent_pe->view_matrix, this->parent_pe->proj_matrix);
            size = in_fr ? size : __FLT_MAX__;

            if (size < this->parent_pe->target_size || nodes[id].count_leaf) {
                selected = true;
                this->cuts_to_submit.push({dealt_points * PipelineStage, cur_id});
                this->parents_to_submit.push(nodes[id].parent_id);
            } else if (nodes[id].subtree_size == 1) {
                this->leaves_to_submit.push({dealt_points * PipelineStage, cur_id});
            }
                
            if (selected || !in_fr) {
                cur_id += nodes[id].subtree_size;
            } else {
                cur_id++;
            }
        }

        this->cur_time = dealt_points * PipelineStage;
        this->counter = 0;

        return true;
    } else if (this->cur_time > 0) {
        this->counter++;

        if (this->counter % PipelineStage == 0) {
            if (!this->cuts_to_submit.empty()) {
                auto cut = this->cuts_to_submit.front();
                int parent_id = this->parents_to_submit.front();
                if (this->counter == cut.first) {
                    this->cuts_to_submit.pop();
                    this->parents_to_submit.pop();

                    this->parent_pe->render_indices.push_back(cut.second);
                    this->parent_pe->parent_indices.push_back(parent_id);
                }
            }

            if (!this->leaves_to_submit.empty()) {
                auto leaf = this->leaves_to_submit.front();
                if (this->counter == leaf.first) {
                    this->leaves_to_submit.pop();
                    
                    // submit leaf_id to scheduler
                }
            }
        }

        if (this->counter == this->cur_time) {
            this->busy = false;
            this->cur_time = 0;
            this->counter = 0;
            this->cur_id = this->inner_id = -1;
            return false;
        }
            
        return true;
    }

    return false;
}