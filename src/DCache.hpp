#ifndef HGS_DCache_HPP_
#define HGS_DCache_HPP_

#include "common.hpp"
#include "DRAM.hpp"
#include "types.hpp"

constexpr int DCacheSize = TaskQueueSize + PENum;

struct CacheData {
    Task task;
    Node node[MaxTaskSize]{};
    Box box[MaxTaskSize]{};
    // actually there should also store the size of each node, but we omit it here
};

struct Bank {
    CacheData data[BankSize];
    bool valid[BankSize] = {false   }; // valid means whether the data has been fully transmitted
    bool occupied[BankSize] = {false}; // occupied means whether the data is being read
    int dram_counter[BankSize] = {0}; // time remaining to finish reading from dram
    int tag[BankSize]{};
    bool busy = false;
    int busy_id = -1;
    int counter = 0; // time remaining to finish reading/loading
};

struct BufferCache {
    CacheData data[BufferCacheSize];
    int tag[BufferCacheSize]{};
    bool valid[BufferCacheSize] = {false};
    bool busy[BufferCacheSize] = {false};
    int counter[BufferCacheSize] = {0};
};

struct DCache {
    Bank banks[BankNum];
    BufferCache buffer_cache;

    bool readData(int task_id, Task &task, std::vector<Node> &nodes, std::vector<Box> &boxes, Box &root_father_box);
    bool readSubtask(int task_id, std::vector<int> &leaves, std::vector<int> &leaf_task_ids);
    bool cacheLoadData(int task_id, const DRAM &dram);
    bool cachePrefillData(int task_id, const DRAM &dram);
    bool bufferCacheLoadData(int task_id, const DRAM &dram);
    std::vector<int> update();
    int invalidate(int task_id);
    void loadBufferCache();
    void printStatus(std::ostream &os);
    bool isBusy();
    void recycle();
    void writeBackSize(int task_id, int node_relevant_id, float size);
};

#endif // HGS_DCache_HPP_