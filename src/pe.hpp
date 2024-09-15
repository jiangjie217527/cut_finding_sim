#ifndef HGS_PE_HPP_
#define HGS_PE_HPP_

#include "common.hpp"
#include "DCache.hpp"
#include "types.hpp"
#include <queue>

struct InnerTask {
    int cycle = 0; // the global cycle
    int inner_id;  // the id of the inner task
    Task cur_task; // the current task it's dealing with
    int cur_id;    // the current node it's dealing with
    int cur_time;  // calculate the time to finish this task
    int counter;   // the passed time
    int offset;    // offset of its working period
    bool busy = false;

    std::vector<std::pair<int, int>> cuts_to_submit;
    std::vector<std::pair<int, int>> leaves_to_submit;

    InnerTask(int offset = -1, int inner_id = -1);
    bool updateTick(std::queue<int> &task_queue, DCache &dcache);
};

struct PE {
    InnerTask inner_tasks[PipelineStage];
    float target_size;
    float* viewpoint;
    std::vector<int>& render_indices;
    std::vector<int>& parent_indices;
    const float* view_matrix;
    const float* proj_matrix;

    int cur_tick = 0;

    bool updateTick(std::queue<int> &task_queue, DCache &dcache);

    PE(std::vector<int>& render_indices,
       std::vector<int>& parent_indices);
    void loadMeta(float target_size, 
                  float* viewpoint,
                  const float* view_matrix,
                  const float* proj_matrix);
};

#endif // HGS_PE_HPP_