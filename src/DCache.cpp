#include "DCache.hpp"

bool DCache::readData(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes) {
    int bank_id = task_id % BankNum;
    
    for (int i = 0; i < BankSize; ++i) {
        if (banks[bank_id].tag[i] == task_id && banks[bank_id].valid[i]) {
            task = banks[bank_id].data[i].task;
            for (int j = 0; j < task.task_size; ++j) {
                nodes.push_back(banks[bank_id].data[i].node[j]);
                boxes.push_back(banks[bank_id].data[i].box[j]);
            }
            return true;
        }
    }

    return false;
}

void DCache::loadData(int task_id, const DRAM &dram) {
    int bank_id = task_id % BankNum;
    
    for (int i = 0; i < BankSize; ++i) {
        if (!banks[bank_id].busy) {
            banks[bank_id].busy = true;
            banks[bank_id].tag[i] = task_id;
            std::vector<Node> nodes;
            std::vector<Box> boxes;
            dram.read(task_id, banks[bank_id].data[i].task, nodes, boxes);
            for (int j = 0; j < banks[bank_id].data[i].task.task_size; ++j) {
                banks[bank_id].data[i].node[j] = nodes[j];
                banks[bank_id].data[i].box[j] = boxes[j];
            }
            banks[bank_id].valid[i] = true;
            banks[bank_id].busy = false;
            return;
        }
    }
}