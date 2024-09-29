#include "DRAM.hpp"
#include <iostream>

void DRAM::read(int task_id, Task &task, std::vector<Node> &read_nodes, std::vector<Box> &read_boxes) const {
  task = tasks[task_id];

  // std::cout << "[debug]: [start, end): [" << task.start_id << ", " << task.start_id + task.task_size << ")\n";

  for (int i = task.start_id; i < task.start_id + task.task_size; ++i) {
    read_nodes.push_back(nodes[i]);
    read_boxes.push_back(boxes[i]);
  }
}

void DRAM::init(const std::vector<Node> &nodes, const std::vector<Task> &tasks, const std::vector<Box> &boxes) {
  this->nodes = nodes;
  this->tasks = tasks;
  this->boxes = boxes;

  std::cout << "[debug]: nodes.size() = " << this->nodes.size() << "\n";
  std::cout << "[debug]: tasks.size() = " << this->tasks.size() << "\n";
  std::cout << "[debug]: boxes.size() = " << this->boxes.size() << "\n";
//    exit(0);
}

void DRAM::print_len(){
	std::cout<<"DRAM print len"<<std::endl;
	std::cout<<nodes.size()<<std::endl<<tasks.size()<<std::endl<<boxes.size()<<std::endl;
	std::cout<<"end DRAM print"<<std::endl;
}
