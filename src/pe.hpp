#ifndef HGS_PE_HPP_
#define HGS_PE_HPP_

#include "common.hpp"
#include "types.hpp"

struct InnerTask {
    int cur_id;   // the current node it's dealing with
    int cur_time; // calculate the time to finish this task
    int counter;  // the passed time
    int offset;   // offset of its working period
    bool cache_enable = false; // whether DCache contains the data
    bool busy = false;

    std::vector<std::pair<int, int>> cuts_to_submit;
    std::vector<std::pair<int, int>> leaves_to_submit;

    InnerTask(int offset = -1);
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

    bool updateTick();

    PE(std::vector<int>& render_indices,
       std::vector<int>& parent_indices);
    void loadMeta(float target_size, 
                  float* viewpoint,
                  const float* view_matrix,
                  const float* proj_matrix);
};

#endif // HGS_PE_HPP_