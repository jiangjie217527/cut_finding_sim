#include "DRAM.hpp"

void DRAM::read(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes) {
    task = tasks[task_id];
    for (int i = task.start_id; i < task.start_id + task.task_size; ++i) {
        nodes.push_back(nodes[i]);
        boxes.push_back(boxes[i]);
    }
}