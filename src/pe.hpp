#ifndef HGS_PE_HPP_
#define HGS_PE_HPP_

#include "common.hpp"
#include "DCache.hpp"
#include "scheduler.hpp"
#include "types.hpp"
#include <queue>

struct PE;

struct InnerTask {
    int cycle = 0; // the global cycle
    int inner_id = -1;  // the id of the inner task
    Task cur_task; // the current task it's dealing with
    int cur_id;    // the current node it's dealing with
    int cur_time;  // calculate the time to finish this task
    int counter;   // the passed time
    int offset;    // offset of its working period
    bool busy = false;

    PE* parent_pe;

    std::queue<std::tuple<int, int, int>> cuts_to_submit; // (time_stamp, nodes_id, start_id)
    std::queue<int> parents_to_submit;
    std::queue<std::tuple<int, int, bool>> leaves_to_submit;

    InnerTask(int offset = -1, int inner_id = -1, PE* parent_pe = nullptr);
    bool updateTick(std::queue<int> &task_queue, DCache &dcache, Scheduler &scheduler);
};

struct PE {
    InnerTask inner_tasks[PipelineStage];
    float target_size;
    float* viewpoint;
    std::vector<int>& render_indices;
    std::vector<int>& nodes_for_render_indices;
    std::vector<int>& parent_indices;
    const float* view_matrix;
    const float* proj_matrix;

    int cur_tick = 0;

    bool updateTick(std::queue<int> &task_queue, DCache &dcache, Scheduler &scheduler);

    PE(std::vector<int>& render_indices,
       std::vector<int>& nodes_for_render_indices,
       std::vector<int>& parent_indices);
    void loadMeta(float target_size, 
                  float* viewpoint,
                  const float* view_matrix,
                  const float* proj_matrix);
};

#endif // HGS_PE_HPP_