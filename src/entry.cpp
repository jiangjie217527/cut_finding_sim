#include "entry.hpp"

Scheduler scheduler;
DRAM dram;
DCache dcache;

std::vector<Task> tasks;
std::vector<Node> nodes;
std::vector<Box> boxes;

void initStage() {
    // todo: load data from binary files
    dram.init(nodes, tasks, boxes);
}

int callAccelerator(float target_size, 
                     float* viewpoint,
                     int* renderIndices,
                     int* parentIndices,
                     const float* view_matrix,
                     const float* proj_matrix) {
    std::vector<int> render_indices, parent_indices;
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
        scheduler.tasks_loaded_to_cache = dcache.update();

        bool working = false;
        for (int i = 0; i < PENum; ++i) {
            working |= pes[i].updateTick(scheduler.task_queue, dcache, scheduler);
        }

        working |= scheduler.schedule(dcache, dram);

        if (!working && scheduler.task_queue.empty()) {
            break;
        }
    }

    for (int i = 0; i < render_indices.size(); ++i) {
        renderIndices[i] = render_indices[i];
        parentIndices[i] = parent_indices[i];
    }

    printf("Total cycles: %d\n", cycle);
    return render_indices.size();
}