#include "common.hpp"
#include "pe.hpp"
#include "types.hpp"

#include <cstdio>
#include <queue>
#include <vector>

void initStage() {

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

    std::queue<int> task_queue;

    while (true) {
        cycle++;
        bool working = false;
        for (int i = 0; i < PENum; ++i) {
            working |= pes[i].updateTick();
        }

        if (!working && task_queue.empty()) {
            break;
        }
    }

    printf("Total cycles: %d\n", cycle);
    return;
}