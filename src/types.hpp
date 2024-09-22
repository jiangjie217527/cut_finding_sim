#ifndef HGS_TYPES_HPP_
#define HGS_TYPES_HPP_

#include <vector>
#include <fstream>

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

    friend std::ifstream& operator>>(std::ifstream& ifs, Task& task) {
    ifs.read(reinterpret_cast<char*>(&task.start_id), sizeof(task.start_id));
    ifs.read(reinterpret_cast<char*>(&task.task_size), sizeof(task.task_size));

    int leaves_size;
    ifs.read(reinterpret_cast<char*>(&leaves_size), sizeof(leaves_size));
    task.leaves.resize(leaves_size);
    ifs.read(reinterpret_cast<char*>(task.leaves.data()), leaves_size * sizeof(int));

    int leaf_task_ids_size;
    ifs.read(reinterpret_cast<char*>(&leaf_task_ids_size), sizeof(leaf_task_ids_size));
    task.leaf_task_ids.resize(leaf_task_ids_size);
    ifs.read(reinterpret_cast<char*>(task.leaf_task_ids.data()), leaf_task_ids_size * sizeof(int));

    if (task.start_id == -1) {
        printf("[INFO] leaves_size: %d\n", leaves_size);
        printf("[INFO] leaf_task_ids_size: %d\n", leaf_task_ids_size);

        for (int i = 0; i < leaves_size; ++i) {
            printf("[INFO] leaves[%d]: %d\n", i, task.leaves[i]);
        }

        for (int i = 0; i < leaf_task_ids_size; ++i) {
            printf("[INFO] leaf_task_ids[%d]: %d\n", i, task.leaf_task_ids[i]);
        }

        printf("\n");
    }

    return ifs;
}

};

struct Box {
    std::vector<float> minn{0, 0, 0, 0};
    std::vector<float> maxx{0, 0, 0, 0};
};

using float4 = std::vector<float>;
using float3 = std::vector<float>;
using Point  = std::vector<float>;

#endif