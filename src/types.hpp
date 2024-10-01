#ifndef HGS_TYPES_HPP_
#define HGS_TYPES_HPP_

#include <Eigen/Dense>
#include <fstream>
#include <vector>

#include "half.hpp"

struct Node {
    int parent_id = 0;
    int subtree_size = 0;
    int count_leaf = 0;
    int start = 0;
    int parent_start = 0;
    float size = 0; // projected size, used for weight calculation
    int num_siblings = 0;
    bool is_task_leaf = false;
};

struct HalfBox {
    half_float::half minn[4];
    half_float::half maxx[4];
};

struct Point4 {
    float xyz[4];

    const float &operator[](int i) const {
      return xyz[i];
    }

    float &operator[](int i) {
      return xyz[i];
    }
};

struct Box {
    Point4 minn;
    Point4 maxx;
};

struct Task {
    int start_id;
    int task_size;
    Box root_father_box; // stored in case we select the father of the root's size
    int root_father_children_num; // stored in case we select the father of the root's size

    std::vector<int> leaves;
    std::vector<int> leaf_task_ids;

    friend std::ifstream &operator>>(std::ifstream &ifs, Task &task) {
      ifs.read(reinterpret_cast<char *>(&task.start_id), sizeof(task.start_id));
      ifs.read(reinterpret_cast<char *>(&task.task_size), sizeof(task.task_size));
      ifs.read(reinterpret_cast<char *>(&task.root_father_box), sizeof(task.root_father_box));
      ifs.read(reinterpret_cast<char *>(&task.root_father_children_num), sizeof(task.root_father_children_num));

      int leaves_size;
      ifs.read(reinterpret_cast<char *>(&leaves_size), sizeof(leaves_size));
      task.leaves.resize(leaves_size);
      ifs.read(reinterpret_cast<char *>(task.leaves.data()), leaves_size * sizeof(int));

      int leaf_task_ids_size;
      ifs.read(reinterpret_cast<char *>(&leaf_task_ids_size), sizeof(leaf_task_ids_size));
      task.leaf_task_ids.resize(leaf_task_ids_size);
      ifs.read(reinterpret_cast<char *>(task.leaf_task_ids.data()), leaf_task_ids_size * sizeof(int));

      return ifs;
    }

};

using float4 = std::vector<float>;
using float3 = std::vector<float>;
typedef Eigen::Vector3f Point;

#endif