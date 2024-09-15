#include "common.hpp"
#include "DCache.hpp"
#include "DRAM.hpp"
#include "pe.hpp"
#include "scheduler.hpp"
#include "types.hpp"

#include <cstdio>
#include <queue>
#include <vector>

Scheduler scheduler;
DRAM dram;
DCache dcache;

std::vector<Task> tasks;
std::vector<Node> nodes;
std::vector<Box> boxes;

void initStage() {
    // todo: load data from binary files
}

void callAccelerator(float target_size, 
                     float* viewpoint,
                     std::vector<int>& render_indices,
                     std::vector<int>& parent_indices,
                     const float* view_matrix,
                     const float* proj_matrix) {
    int cycle = 0;
    PE pes[PENum] = {
        PE(render_indices, parent_indices),
        PE(render_indices, parent_indices),
        PE(render_indices, parent_indices),
        PE(render_indices, parent_indices)
    };
    for (int i = 0; i < PENum; ++i) {
        pes[i].loadMeta(target_size, viewpoint, view_matrix, proj_matrix);
    }

    scheduler.task_queue.push(0);
    

    while (true) {
        cycle++;
        bool working = false;
        for (int i = 0; i < PENum; ++i) {
            working |= pes[i].updateTick(scheduler.task_queue, dcache);
        }

        if (!working && scheduler.task_queue.empty()) {
            break;
        }
    }

    printf("Total cycles: %d\n", cycle);
    return;
}