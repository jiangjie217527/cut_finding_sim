#ifndef HGS_TYPES_HPP_
#define HGS_TYPES_HPP_

#include <vector>

struct Node {
    int parent_id;
    int subtree_size;
    int count_leaf;
};

struct Task {
    int start_id;
    int task_size;
    std::vector<int> leaves;
    std::vector<int> leaf_task_ids;
};

struct Box {
    std::vector<int> minn{0, 0, 0, 0};
    std::vector<int> maxx{0, 0, 0, 0};
};

#endif