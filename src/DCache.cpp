#include "DCache.hpp"
#include "common.hpp"

int divUpperBound(int a, int b) {
    return (a + b - 1) / b;
}

bool DCache::readData(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes) {
    int bank_id = task_id % BankNum;
    
    if (banks[bank_id].busy) {
        return false;
    }

    for (int i = 0; i < BankSize; ++i) {
        if (banks[bank_id].tag[i] == task_id && banks[bank_id].valid[i]) {
            task = banks[bank_id].data[i].task;
            for (int j = 0; j < task.task_size; ++j) {
                nodes.push_back(banks[bank_id].data[i].node[j]);
                boxes.push_back(banks[bank_id].data[i].box[j]);
            }

            banks[bank_id].busy = true;
            banks[bank_id].counter = 1;
            return true;
        }
    }

    return false;
}

bool DCache::readSubtask(int task_id, std::vector<int> &leaves, std::vector<int> &leaf_task_ids) {
    int bank_id = task_id % BankNum;

    if (banks[bank_id].busy) {
        return false;
    }
    
    for (int i = 0; i < BankSize; ++i) {
        if (banks[bank_id].tag[i] == task_id && banks[bank_id].valid[i]) {
            leaves = banks[bank_id].data[i].task.leaves;
            leaf_task_ids = banks[bank_id].data[i].task.leaf_task_ids;

            banks[bank_id].busy = true;
            banks[bank_id].counter = divUpperBound((leaves.size() + leaf_task_ids.size()) * sizeof(int), CacheWordsPerCycle);
            return true;
        }
    }

    return false;
}

void DCache::loadData(int task_id, const DRAM &dram) {
    int bank_id = task_id % BankNum;
    
    for (int i = 0; i < BankSize; ++i) {
        banks[bank_id].tag[i] = task_id;
        std::vector<Node> nodes;
        std::vector<Box> boxes;
        dram.read(task_id, banks[bank_id].data[i].task, nodes, boxes);
        for (int j = 0; j < banks[bank_id].data[i].task.task_size; ++j) {
            banks[bank_id].data[i].node[j] = nodes[j];
            banks[bank_id].data[i].box[j] = boxes[j];
        }
        banks[bank_id].valid[i] = true;
        return;
    }
}

void DCache::update() {
    for (int i = 0; i < BankNum; ++i) {
        if (banks[i].busy) {
            banks[i].counter--;
            if (banks[i].counter == 0) {
                banks[i].busy = false;
            }
        }
    }
}