#include "entry.hpp"
#include <cstdio>
#include <iostream>

Scheduler scheduler;
DRAM dram;
DCache dcache;

constexpr int maxn = 3e6;
int renderIndices[maxn], parentIndices[maxn];

std::vector<Task> tasks;
std::vector<Node> nodes;
std::vector<Box> boxes;
std::vector<int> reorder_indices;
std::vector<int> reversed_indices;

void initStage() {
  std::ifstream infile("reorder copy.bin", std::ios::binary);
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
  }

  size_t nodes_size;
  infile.read(reinterpret_cast<char *>(&nodes_size), sizeof(nodes_size));
  nodes.resize(nodes_size);
  boxes.resize(nodes_size);
  for (int i = 0; i < nodes_size; ++i) {
    int parent, subtree_size, count_leaf;
    infile.read(reinterpret_cast<char *>(&parent), sizeof(parent));
    infile.read(reinterpret_cast<char *>(&subtree_size), sizeof(subtree_size));
    infile.read(reinterpret_cast<char *>(&count_leaf), sizeof(count_leaf));
    nodes[i] = {parent, subtree_size, count_leaf};

    Point4 minn, maxx;
    infile.read(reinterpret_cast<char *>(&minn), sizeof(minn));
    infile.read(reinterpret_cast<char *>(&maxx), sizeof(maxx));

    for (int j = 0; j < 4; ++j) {
      boxes[i].minn[j] = minn[j];
      boxes[i].maxx[j] = maxx[j];
    }
  }

  infile.close();

  dram.init(nodes, tasks, boxes);
}

int callAccelerator(float target_size,
                    float *viewpoint,
                    int *renderIndices,
                    int *parentIndices,
                    const float *view_matrix,
                    const float *proj_matrix) {
  initStage();

  printf("Start calling accelerator\n");

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
  dcache.cachePrefillData(0, dram);

  while (cycle < 10000) {
    cycle++;
    printf("Cycle: %d\n", cycle);
    scheduler.tasks_loaded_to_cache = dcache.update();

    for (auto &loaded_task: scheduler.tasks_loaded_to_cache) {

    }

    for (auto &loaded_task: scheduler.tasks_loaded_to_cache) {
      scheduler.task_queue.push(loaded_task);
    }

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

  for (int i = 0; i < render_indices.size(); ++i) {
    renderIndices[i] = reversed_indices[renderIndices[i]];
    parentIndices[i] = reversed_indices[parentIndices[i]];
  }

  return render_indices.size();
}

int main() {
  float target_size = 0.015236032862841614;
  float viewpoint[3] = {-1.1579, 19.6893, -2.9418};
  float view_matrix[16] = {0.95878, 0.10376, -0.26453, 0.0,
                           0.28411, -0.33513, 0.89831, 0.0,
                           0.0045524, -0.93644, -0.35080, 0.0,
                           -4.4704, 3.9639, -19.025, 1.0};
  float proj_matrix[16] = {1.0055, 0.0, 0.0, 0.0,
                           0.0, 1.3369, 0.0, 0.0,
                           0.0, 0.0, 1.0001, 1.0,
                           0.0, 0.0, -0.0100, 0.0};

  freopen("log.txt", "w", stdout);
  std::cout << callAccelerator(target_size, viewpoint, renderIndices, parentIndices, view_matrix, proj_matrix)
            << std::endl;
  fclose(stdout);
  return 0;
}