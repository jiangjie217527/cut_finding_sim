#include "pe.hpp"
#include "common.hpp"
#include "utils.hpp"

InnerTask::InnerTask(int offset, int inner_id, PE *parent_pe) : inner_id(inner_id), offset(offset) {
  this->parent_pe = parent_pe;
  this->cur_id = this->inner_id = -1;
  this->cur_time = this->counter = 0;
  this->cycle = 0;
  this->busy = false;
}

PE::PE(std::vector<int> &render_indices,
       std::vector<int> &nodes_for_render_indices,
       std::vector<int> &parent_indices,
       std::vector<float> &ts,
       std::vector<int> &kids) : render_indices(render_indices),
                                 nodes_for_render_indices(nodes_for_render_indices),
                                 parent_indices(parent_indices),
                                 ts(ts),
                                 kids(kids) {
  for (int i = 0; i < PipelineStage; ++i) {
    inner_tasks[i] = InnerTask(i, i, this);
  }
}

bool PE::updateTick(std::queue<int> &task_queue, DCache &dcache, Scheduler &scheduler) {
  bool busy = false;
  for (auto & inner_task : inner_tasks) {
    busy |= inner_task.updateTick(task_queue, dcache, scheduler);
  }

  return busy;
}

bool InnerTask::updateTick(std::queue<int> &task_queue, DCache &dcache, Scheduler &scheduler) {
//    printf("[debug]: cycle = %d, offset = %d\n", cycle, offset);
  this->cycle++;

  if (!busy) {
    if (cycle % PipelineStage != offset) {
      return this->inner_id != -1;
    }

    if (task_queue.empty() && this->inner_id == -1) {
      return false;
    }

    if (this->inner_id == -1) {
      this->inner_id = task_queue.front();
      task_queue.pop();
    }

    std::vector<Node> nodes;
    std::vector<Box> boxes;
    Box root_father_box{};

    if (!dcache.readData(inner_id, this->cur_task, nodes, boxes, root_father_box)) {
      return true; // bank conflict
    }

    this->busy = true;
    this->cur_id = cur_task.start_id;

    int dealt_points = 0;

    while (cur_id < cur_task.start_id + cur_task.task_size && cur_id >= cur_task.start_id) {

      dealt_points++;

      int id = cur_id - cur_task.start_id;
      Point viewpoint = {this->parent_pe->viewpoint[0], this->parent_pe->viewpoint[1], this->parent_pe->viewpoint[2]};
      float size = computeSize(boxes[id], viewpoint);
      nodes[id].size = size;
      bool selected = false, in_fr = in_frustum(boxes[id], this->parent_pe->view_matrix, this->parent_pe->proj_matrix);

      if (((size < this->parent_pe->target_size && size > 0) || nodes[id].count_leaf) && in_fr) { // select this point!
        selected = true;
        this->cuts_to_submit.emplace((dealt_points -1) * PipelineStage + CutSelectStage, cur_id, nodes[id].start);
        this->parents_to_submit.push(nodes[id].parent_start);

        int parent_id = nodes[id].parent_id - cur_task.start_id;
        float weight;
        Box parent_box = parent_id >= 0 ? boxes[parent_id] : root_father_box;
        int num_kids = parent_id >= 0 ? nodes[id].num_siblings : cur_task.root_father_children_num;
        float parent_size = computeSize(parent_box, viewpoint);
        if (parent_size > 2.0f * this->parent_pe->target_size) {
          weight = 1.0f;
        } else {
          float start = std::max(0.5f * parent_size, size);
          float diff = parent_size - start;
          assert(diff > 0);
          weight = std::max(1.0f - (std::max(0.0f, this->parent_pe->target_size - start) / diff), 0.0f);
        }

        this->weight_to_submit.emplace(dealt_points * PipelineStage, weight, num_kids);
      } else if (nodes[id].is_task_leaf) {
        this->leaves_to_submit.emplace((dealt_points -1) * PipelineStage + CutSelectStage, cur_id,
                                       cur_id + nodes[id].subtree_size >= cur_task.start_id + cur_task.task_size);
      }

      // write your size back to DCache
      dcache.writeBackSize(inner_id, id, size);

      if (selected || !in_fr) {
        cur_id += nodes[id].subtree_size;
      } else {
        cur_id++;
      }
    }

    std::queue<std::tuple<int, int, bool>> tmp;
    while (!leaves_to_submit.empty()) {
      if (leaves_to_submit.size() == 1) {
        tmp.emplace(std::get<0>(leaves_to_submit.front()), std::get<1>(leaves_to_submit.front()), true);
      } else {
        tmp.emplace(std::get<0>(leaves_to_submit.front()), std::get<1>(leaves_to_submit.front()), false);
      }

      leaves_to_submit.pop();
    }

    leaves_to_submit = tmp;

    if (leaves_to_submit.empty()) {
      leaves_to_submit.emplace(dealt_points * PipelineStage, -1, true);
    }

    this->cur_time = dealt_points * PipelineStage;
    this->counter = 0;

    return true;
  } else if (this->cur_time > 0) {
    this->counter++;

    if (this->counter % PipelineStage == CutSelectStage) {
      if (!this->cuts_to_submit.empty()) {
        auto cut = this->cuts_to_submit.front();
        int time_stamp = std::get<0>(cut), node_id = std::get<1>(cut), start_id = std::get<2>(cut);
        int parent_id = this->parents_to_submit.front();
        if (this->counter == time_stamp) {
          this->cuts_to_submit.pop();
          this->parents_to_submit.pop();

          this->parent_pe->render_indices.push_back(start_id);
          this->parent_pe->nodes_for_render_indices.push_back(node_id);
          this->parent_pe->parent_indices.push_back(parent_id);
        }
      }

      if (!this->leaves_to_submit.empty()) {
        auto leaf = this->leaves_to_submit.front();
        int time_stamp = std::get<0>(leaf);
        int leaf_id = std::get<1>(leaf);
        bool is_end = std::get<2>(leaf);

        if (this->counter >= time_stamp && scheduler.leaf_to_submit.size() < MaxLeafBufferSize) {
          this->leaves_to_submit.pop();

          // submit leaf_id to scheduler
          scheduler.leaf_to_submit.emplace(this->inner_id, this->cycle, leaf_id, is_end);
        }
      }
    }

    if (this->counter % PipelineStage == 0) {
      if (!this->weight_to_submit.empty()) {
        auto weight = this->weight_to_submit.front();
        int time_stamp = std::get<0>(weight);
        float w = std::get<1>(weight);
        int children_num = std::get<2>(weight);

        if (this->counter == time_stamp) {
          this->weight_to_submit.pop();
          this->parent_pe->ts.push_back(w);
          this->parent_pe->kids.push_back(children_num);
        }
      }
    }

    if (this->counter >= this->cur_time && this->cuts_to_submit.empty() && this->leaves_to_submit.empty()) {
      scheduler.tasks_to_submit.emplace(this->cycle, this->inner_id);

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

void PE::loadMeta(float _target_size,
                  float *_viewpoint,
                  const float *_view_matrix,
                  const float *_proj_matrix) {
  this->target_size = _target_size;
  this->viewpoint = _viewpoint;
  this->view_matrix = _view_matrix;
  this->proj_matrix = _proj_matrix;
}

bool PE::isBusy() {
  for (const auto & inner_task : inner_tasks) {
    if (inner_task.isBusy()) {
      return true;
    }
  }

  return false;
}

bool InnerTask::isBusy() const {
  if (this->busy) {
    return true;
  }

  return this->inner_id != -1;
}
