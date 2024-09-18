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
    std::vector<float> minn{0, 0, 0, 0};
    std::vector<float> maxx{0, 0, 0, 0};
};

using float4 = std::vector<float>;
using float3 = std::vector<float>;
using Point  = std::vector<float>;

#endif