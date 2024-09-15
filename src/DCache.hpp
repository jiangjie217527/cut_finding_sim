#ifndef HGS_DCache_HPP_
#define HGS_DCache_HPP_

#include "common.hpp"
#include "DRAM.hpp"
#include "types.hpp"

constexpr int DCacheSize = TaskQueueSize + PENum;

struct CacheData {
    Task task;
    Node node[MaxTaskSize];
    Box box[MaxTaskSize];
};

struct Bank {
    CacheData data[BankSize];
    bool valid[BankSize] = {false   };
    int tag[BankSize];
    bool busy = false;
};

struct DCache {
    Bank banks[BankNum];

    bool readData(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes);
    void loadData(int task_id, const DRAM &dram);
};

#endif // HGS_DCache_HPP_