#ifndef HGS_DRAM_HPP_
#define HGS_DRAM_HPP_

#include "types.hpp"
#include <tuple>

struct DRAM {
    std::vector<Node> nodes;
    std::vector<Task> tasks;
    std::vector<Box> boxes;

    DRAM() = default;

    DRAM(const std::vector<Node> &nodes,
         const std::vector<Task> &tasks,
         const std::vector<Box> &boxes) : nodes(nodes), tasks(tasks), boxes(boxes) {}

    void read(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes) const;

    void init(const std::vector<Node> &nodes, const std::vector<Task> &tasks, const std::vector<Box> &boxes);

    void print_len();
};

#endif // HGS_DRAM_HPP_
