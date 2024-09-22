#include "DRAM.hpp"
#include <iostream>

void DRAM::read(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes) const {
    task = tasks[task_id];

    std::cout << "[debug]: [start, end): [" << task.start_id << ", " << task.start_id + task.task_size << ")\n";

    for (int i = task.start_id; i < task.start_id + task.task_size; ++i) {
        nodes.push_back(nodes[i]);
        boxes.push_back(boxes[i]);
    }
}

void DRAM::init(const std::vector<Node>& nodes, const std::vector<Task>& tasks, const std::vector<Box>& boxes) {
    this->nodes = nodes;
    this->tasks = tasks;
    this->boxes = boxes;
}