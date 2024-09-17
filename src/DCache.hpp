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
    int counter = 0; // time remaining to finish reading/loading
};

struct BufferCache {
    CacheData data[BufferCacheSize];
    int tag[BufferCacheSize];
    bool valid[BufferCacheSize] = {false};

    bool busy = false;
    int counter = 0;
    
};

struct DCache {
    Bank banks[BankNum];
    BufferCache buffer_cache;

    bool readData(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes);
    bool readSubtask(int task_id, std::vector<int> &leaves, std::vector<int> &leaf_task_ids);
    void loadData(int task_id, const DRAM &dram);
    void update();
};

#endif // HGS_DCache_HPP_