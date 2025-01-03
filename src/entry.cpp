#include "entry.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>

Scheduler scheduler;
DRAM dram;
DCache dcache;

constexpr int maxn = 8e6;
int globalNodesForRenderIndices[maxn], globalParentIndices[maxn], globalRenderIndices[maxn], globalKids[maxn];
float globalTS[maxn];

struct FinishInformation {
    int node_for_render_index;
    int parent_index;
    int render_index;
    float weight;
    int children_num;

    friend bool operator<(const FinishInformation &a, const FinishInformation &b) {
      return a.node_for_render_index < b.node_for_render_index;
    }
} finishInformation[maxn];

std::vector<Task> tasks;
std::vector<Node> nodes;
std::vector<Box> boxes;
std::vector<int> reorder_indices;
std::vector<int> reversed_indices;
std::unordered_set<int> render_set;

void multi() {
  std::cout << "render_set.size:" << render_set.size() << std::endl;
  std::cout << "tasks.size:" << tasks.size() << std::endl;
  scheduler.print_len();
  dram.print_len();
}

void recycle() {
  render_set.clear();
  scheduler.recycle();
  dcache.recycle();
}

void check(std::string truth_file, std::string my_file) {

  freopen(truth_file.c_str(), "r", stdin);
  std::vector<float> truth;
  while (true) {
    int x;
    if (scanf("%d", &x) == EOF) {
      break;
    }
    truth.push_back(x);
  }

  freopen(my_file.c_str(), "r", stdin);
  std::vector<float> mine;

  while (true) {
    float x;
    if (scanf("%f", &x) == EOF) {
      break;
    }
    mine.push_back(x);
  }


  std::sort(truth.begin(), truth.end());
  std::sort(mine.begin(), mine.end());

  int overlap = 0;
  std::unordered_set<float> truth_nodes;

  for (int i = 0; i < truth.size(); ++i) {
    truth_nodes.insert(truth[i]);
  }

  for (int i = 0; i < mine.size(); ++i) {
    if (truth_nodes.find(mine[i]) != truth_nodes.end()) {
      overlap++;
    } else {
      std::cerr << "[ERROR]: mine[" << i << "] = " << mine[i] << " not found by truth\n";
      exit(1);
    }
  }

  std::cerr << "[INFO]: overlap rate = " << overlap << " / " << mine.size() << " = " << overlap * 1.0 / mine.size()
            << "\n";
}

void checkNodeOverlap() {
  std::unordered_set<int> indices;
  for (int i = 0; i < tasks.size(); ++i) {
    Task task = tasks[i];
    for (int j = task.start_id; j < task.start_id + task.task_size; ++j) {
      if (indices.find(j) != indices.end()) {
        printf("Error: overlap\n");
        exit(1);
      }
      indices.insert(j);
    }
  }
}

void checkTaskOverlap() {
  std::unordered_set<int> task_set;

  for (int i = 0; i < tasks.size(); ++i) {
    Task task = tasks[i];
    for (int j = 0; j < task.leaves.size(); ++j) {
      int subtask = task.leaf_task_ids[j];
      if (task_set.find(subtask) != task_set.end()) {
        std::cerr << "[ERROR]: duplicate subtask: " << subtask << "\n";
        exit(1);
      }

      task_set.insert(subtask);
    }
  }

  std::cerr << "[INFO]: pass checkTaskOverlap\n";
}

void checkReorder() {
  for (int i = 0; i < reorder_indices.size(); ++i) {
    assert(reversed_indices[reorder_indices[i]] == i);
  }
}

void checkOverlap() {
  checkReorder();
  std::cerr << "[INFO]: pass checkReorder\n";
  checkNodeOverlap();
  checkTaskOverlap();
}

void initStage(float *viewpoint) {
  if (!tasks.empty()) {
    std::cout << "[reuse]: tasks.size() = " << tasks.size() << '\n';

    return;
  }

  size_t max_subtask_size = 0;

  std::ifstream infile("reorder.bin", std::ios::binary);
  if (!infile.is_open()) {
    printf("Error: cannot open reorder.bin\n");
    exit(1);
  }

  printf("Loading reorder.bin\n");

  size_t reorder_size;
  infile.read(reinterpret_cast<char *>(&reorder_size), sizeof(reorder_size));
  reorder_indices.resize(reorder_size);
  infile.read(reinterpret_cast<char *>(reorder_indices.data()), reorder_size * sizeof(int));

  printf("[INFO] reorder_size: %ld\n", reorder_size);

  size_t reversed_size;
  infile.read(reinterpret_cast<char *>(&reversed_size), sizeof(reversed_size));
  reversed_indices.resize(reversed_size);
  infile.read(reinterpret_cast<char *>(reversed_indices.data()), reversed_size * sizeof(int));

  printf("[INFO] reversed_size: %ld\n", reversed_size);

  size_t tasks_size;
  infile.read(reinterpret_cast<char *>(&tasks_size), sizeof(tasks_size));

  printf("[INFO] tasks_size: %ld\n", tasks_size);

  tasks.resize(tasks_size);
  for (int i = 0; i < tasks_size; ++i) {
    infile >> tasks[i];
    max_subtask_size = std::max(max_subtask_size, tasks[i].leaf_task_ids.size());
  }

  for (int i = 0; i < tasks_size; ++i) {
    readExtraData(infile, tasks[i]);
  }

  std::cerr << "[critical]: max_subtask_size = " << max_subtask_size << "\n";

  size_t nodes_size;
  infile.read(reinterpret_cast<char *>(&nodes_size), sizeof(nodes_size));
  nodes.resize(nodes_size);
  boxes.resize(nodes_size);
  for (int i = 0; i < nodes_size; ++i) {
    int parent, subtree_size, count_leaf, start, num_siblings;
    infile.read(reinterpret_cast<char *>(&parent), sizeof(parent));
    infile.read(reinterpret_cast<char *>(&subtree_size), sizeof(subtree_size));
    infile.read(reinterpret_cast<char *>(&count_leaf), sizeof(count_leaf));
    infile.read(reinterpret_cast<char *>(&start), sizeof(start));
    infile.read(reinterpret_cast<char *>(&num_siblings), sizeof(num_siblings));
    nodes[i] = {parent, subtree_size, count_leaf, start, 0, false, 0, num_siblings};

    Point4 minn, maxx;
    infile.read(reinterpret_cast<char *>(&minn), sizeof(minn));
    infile.read(reinterpret_cast<char *>(&maxx), sizeof(maxx));

    for (int j = 0; j < 4; ++j) {
      boxes[i].minn[j] = minn[j];
      boxes[i].maxx[j] = maxx[j];
    }
  }

  for (int i = 0; i < nodes_size; ++i) {
    nodes[i].parent_start = nodes[nodes[i].parent_id].start;
  }

  infile.close();

  for (int i = 0; i < tasks_size; ++i) {
    Task &task = tasks[i];
    for (int j = 0; j < task.leaves.size(); ++j) {
      int leaf = task.leaves[j];
      nodes[leaf].is_task_leaf = true;
    }
  }

  std::cerr << "[INFO] tasks.size() = " << tasks.size() << "\n";
  std::cerr << "[INFO] nodes.size() = " << nodes.size() << "\n";

  dram.init(nodes, tasks, boxes);
}

std::pair<int, int> callAccelerator(float target_size,
                    float *viewpoint,
                    int *renderIndices,
                    int *nodesForRenderIndices,
                    int *parentIndices,
                    float *TS,
                    int *Kids,
                    const float *view_matrix,
                    const float *proj_matrix) {
  initStage(viewpoint);

  printf("Start calling accelerator\n");

  std::vector<int> render_indices, nodes_for_render_indices, parent_indices;
  std::vector<float> ts;
  std::vector<int> kids;
  int cycle = 0;
  PE pes[PENum] = {
          PE(render_indices, nodes_for_render_indices, parent_indices, ts, kids),
          PE(render_indices, nodes_for_render_indices, parent_indices, ts, kids),
          PE(render_indices, nodes_for_render_indices, parent_indices, ts, kids),
          PE(render_indices, nodes_for_render_indices, parent_indices, ts, kids),
  };

  double occupy_rate[PENum][PipelineStage];
  for (int i = 0; i < PENum; ++i) {
    for (int j = 0; j < PipelineStage; ++j) {
      occupy_rate[i][j] = 0;
    }
  }

  for (int i = 0; i < PENum; ++i) {
    pes[i].loadMeta(target_size, viewpoint, view_matrix, proj_matrix);
  }

  scheduler.task_queue.push(0);
  dcache.cachePrefillData(0, dram);

  size_t prev = 0;
  while (true) {
    cycle++;
    if (cycle % 100000 == 0) {
      printf("Cycle: %d\n", cycle);
    }
    scheduler.tasks_loaded_to_cache = dcache.update();

    for (auto &loaded_task: scheduler.tasks_loaded_to_cache) {
      scheduler.task_queue.push(loaded_task);
    }

    bool working = false;

    scheduler.schedule(dcache, dram);

    for (int i = 0; i < PENum; ++i) {
      pes[i].updateTick(scheduler.task_queue, dcache, scheduler);
    }

    working |= dcache.isBusy();

    for (int i = 0; i < PENum; ++i) {
      working |= pes[i].isBusy();

      for (int j = 0; j < PipelineStage; ++j) {
        occupy_rate[i][j] += pes[i].inner_tasks[j].busy;
      }
    }

    working |= scheduler.isBusy();

    if (!working) {
      break;
    }

    for (size_t i = prev; i < nodes_for_render_indices.size(); ++i) {
      if (render_set.find(nodes_for_render_indices[i]) != render_set.end()) {
        std::cerr << "[ERROR] duplicate nodes_for_render_indices: " << nodes_for_render_indices[i] << "\n";
        exit(1);
      }

      render_set.insert(nodes_for_render_indices[i]);
    }

    prev = nodes_for_render_indices.size();
  }
  // make sure nodes_for_render_indices is unique

  for (int i = 0; i < nodes_for_render_indices.size(); ++i) {
    renderIndices[i] = render_indices[i];
    nodesForRenderIndices[i] = nodes_for_render_indices[i];
    parentIndices[i] = parent_indices[i];
    TS[i] = ts[i];
    Kids[i] = kids[i];
  }

  std::cerr << "total cycles: " << cycle << std::endl;

  std::cerr << "occupy rate: \n";
  for (int i = 0; i < PENum; ++i) {
    std::cerr << "PE " << i << ": \n";
    for (int j = 0; j < PipelineStage; ++j) {
      std::cerr << occupy_rate[i][j] * 1.0 / cycle << ", ";
    }

    std::cerr << "\n";
  }

  for (int i = 0; i < nodes_for_render_indices.size(); ++i) {
    nodesForRenderIndices[i] = reversed_indices[nodesForRenderIndices[i]];
    finishInformation[i] = {nodesForRenderIndices[i], parent_indices[i], render_indices[i], ts[i], kids[i]};
  }

  recycle();

  return {nodes_for_render_indices.size(), cycle};
}

int main() {
  float target_size = 0.015236032862841614;
  float viewpoint[3] = {-1.1579, 19.6893, -2.9418};
  float view_matrix[16] = {0.95878, 0.103756, -0.264529, 0.0,
                           0.284113, -0.335135, 0.898312, 0.0,
                           0.00455242, -0.93644, -0.350799, 0.0,
                           -4.4704, 3.9639, -19.0254, 1.0};
  float proj_matrix[16] = {1.00553, 0.0, 0.0, 0.0,
                           0.0, 1.33695, 0.0, 0.0,
                           0.0, 0.0, 1.0001, 1.0,
                           0.0, 0.0, -0.010001, 0.0};

  std::pair<int, int> res = callAccelerator(target_size, viewpoint, globalRenderIndices, globalNodesForRenderIndices,
                                  globalParentIndices, globalTS, globalKids, view_matrix, proj_matrix);
  int to_render = res.first, cycles = res.second;
  freopen("log.txt", "w", stdout);
  std::cerr << to_render << std::endl;
  freopen("my_render_indices.txt", "w", stdout);
  for (int i = 0; i < to_render; ++i) {
    std::cout << globalRenderIndices[i] << '\n';
  }

  freopen("my_nodes_to_render.txt", "w", stdout);
  for (int i = 0; i < to_render; ++i) {
    std::cout << globalNodesForRenderIndices[i] << '\n';
  }

  freopen("my_parents.txt", "w", stdout);
  for (int i = 0; i < to_render; ++i) {
    std::cout << globalParentIndices[i] << '\n';
  }

  freopen("my_interpolation_weights.txt", "w", stdout);
  for (int i = 0; i < to_render; ++i) {
    std::cout << globalTS[i] << '\n';
  }

  freopen("my_num_siblings.txt", "w", stdout);
  for (int i = 0; i < to_render; ++i) {
    std::cout << globalKids[i] << '\n';
  }

  check("./render_indices.txt", "my_render_indices.txt");
  check("./nodes_for_render_indices.txt", "my_nodes_to_render.txt");
  check("./parent_indices.txt", "my_parents.txt");
  return 0;
}
