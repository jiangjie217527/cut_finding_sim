#include "DCache.hpp"
#include "common.hpp"
#include <cassert>

int divUpperBound(int a, int b) {
  return (a + b - 1) / b;
}

bool DCache::busy() {
  for (int i = 0; i < BankNum; ++i) {
    for (int j = 0; j < BankSize; ++j) {
      if (banks[i].occupied[j]) {
        return true;
      }
    }
  }

  for (int i = 0; i < BufferCacheSize; ++i) {
    if (buffer_cache.busy[i]) {
      return true;
    }
  }

  return false;
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

bool DCache::cachePrefillData(int task_id, const DRAM &dram) {
  int bank_id = task_id % BankNum;

  for (int i = 0; i < BankSize; ++i) {
    if (banks[bank_id].valid[i] || banks[bank_id].occupied[i]) {
      continue;
    }

    banks[bank_id].tag[i] = task_id;
    std::vector<Node> nodes;
    std::vector<Box> boxes;

    dram.read(task_id, banks[bank_id].data[i].task, nodes, boxes);
    for (int j = 0; j < banks[bank_id].data[i].task.task_size; ++j) {
      banks[bank_id].data[i].node[j] = nodes[j];
      banks[bank_id].data[i].box[j] = boxes[j];
    }
    banks[bank_id].valid[i] = true;
    banks[bank_id].occupied[i] = false;
    banks[bank_id].dram_counter[i] = divUpperBound(SizeOfCacheData, CacheWordsPerCycle);
    return true;
  }

  return false;
}

bool DCache::cacheLoadData(int task_id, const DRAM &dram) {
  int bank_id = task_id % BankNum;

  for (int i = 0; i < BankSize; ++i) {
    if (banks[bank_id].valid[i] || banks[bank_id].occupied[i]) {
      continue;
    }

    banks[bank_id].tag[i] = task_id;
    std::vector<Node> nodes;
    std::vector<Box> boxes;

    dram.read(task_id, banks[bank_id].data[i].task, nodes, boxes);
    for (int j = 0; j < banks[bank_id].data[i].task.task_size; ++j) {
      banks[bank_id].data[i].node[j] = nodes[j];
      banks[bank_id].data[i].box[j] = boxes[j];
    }
    banks[bank_id].valid[i] = false;
    banks[bank_id].occupied[i] = true;
    banks[bank_id].dram_counter[i] = divUpperBound(SizeOfCacheData, DRAMWordsPerCycle);
    return true;
  }

  return false;
}

bool DCache::bufferCacheLoadData(int task_id, const DRAM &dram) {
  for (int i = 0; i < BufferCacheSize; ++i) {
    if (buffer_cache.valid[i] || buffer_cache.busy[i]) {
      continue;
    }

    buffer_cache.tag[i] = task_id;
    std::vector<Node> nodes;
    std::vector<Box> boxes;

    dram.read(task_id, buffer_cache.data[i].task, nodes, boxes);
    for (int j = 0; j < buffer_cache.data[i].task.task_size; ++j) {
      buffer_cache.data[i].node[j] = nodes[j];
      buffer_cache.data[i].box[j] = boxes[j];
    }
    buffer_cache.valid[i] = false;
    buffer_cache.busy[i] = true;
    buffer_cache.counter[i] = divUpperBound(SizeOfCacheData, DRAMWordsPerCycle);
    return true;
  }

  return false;
}

std::vector<int> DCache::update() {
  std::vector<int> res;
  for (int i = 0; i < BankNum; ++i) {
    if (banks[i].busy) { // data transfer from buffer cache
      banks[i].counter--;
      if (banks[i].counter == 0) {
        // finish transferring data
        banks[i].busy = false;
        if (banks[i].busy_id != -1) {
          banks[i].valid[banks[i].busy_id] = true;
          banks[i].occupied[banks[i].busy_id] = false;
          res.push_back(banks[i].tag[banks[i].busy_id]);
        }
      }
    }

    for (int j = 0; j < BankSize; ++j) { // data transfer from DRAM
      if (banks[i].occupied[j]) {
        banks[i].dram_counter[j]--;
        if (banks[i].dram_counter[j] == 0) {
          banks[i].occupied[j] = false;
          banks[i].valid[j] = true;
          res.push_back(banks[i].tag[j]);
        }
      }
    }
  }

  return res;
}

int DCache::invalidate(int task_id) {
  int bank_id = task_id % BankNum;

  for (int i = 0; i < BankSize; ++i) {
    if (banks[bank_id].tag[i] == task_id && banks[bank_id].valid[i]) {
      banks[bank_id].valid[i] = false;
      return i;
    }
  }

  assert(false);
}

void DCache::loadBufferCache() {
  for (int i = 0; i < BufferCacheSize; ++i) {
    if (!buffer_cache.valid[i]) {
      continue;
    }

    int bank_id = buffer_cache.tag[i] % BankNum;
    if (banks[bank_id].busy) {
      continue;
    }

    // now we're sure that the bank isn't busy, can fill the data
    for (int j = 0; j < BankSize; ++j) {
      if (!banks[bank_id].valid[j] && !banks[bank_id].occupied[j]) {
        banks[bank_id].tag[j] = buffer_cache.tag[i];
        banks[bank_id].data[j] = buffer_cache.data[i];
        banks[bank_id].occupied[j] = true;
        banks[bank_id].busy = true;
        banks[bank_id].busy_id = j;
        banks[bank_id].counter = divUpperBound(SizeOfCacheData, CacheWordsPerCycle);
        buffer_cache.valid[i] = false;
        buffer_cache.busy[i] = false;
        buffer_cache.counter[i] = 0;

        break;
      }
    }
  }
}

// print the layout of DCache, containing banks and the buffer cache, and the status, tags of each bank and the buffer cache
void DCache::printStatus() {
  std::cout << "DCache status:\n";
  for (int i = 0; i < BankNum; ++i) {
    std::cout << "Bank " << i << ":\n";
    for (int j = 0; j < BankSize; ++j) {
      std::cout << "  " << j << ": ";
      if (banks[i].valid[j]) {
        std::cout << "valid, ";
      } else {
        std::cout << "invalid, ";
      }

      if (banks[i].occupied[j]) {
        std::cout << "occupied, ";
      } else {
        std::cout << "not occupied, ";
      }

      if (banks[i].busy) {
        std::cout << "busy, ";
      } else {
        std::cout << "not busy, ";
      }

      std::cout << "tag = " << banks[i].tag[j] << ", ";
      std::cout << "dram_counter = " << banks[i].dram_counter[j] << "\n";
    }
  }

  std::cout << "Buffer Cache:\n";
  for (int i = 0; i < BufferCacheSize; ++i) {
    std::cout << "  " << i << ": ";
    if (buffer_cache.valid[i]) {
      std::cout << "valid, ";
    } else {
      std::cout << "invalid, ";
    }

    if (buffer_cache.busy[i]) {
      std::cout << "busy, ";
    } else {
      std::cout << "not busy, ";
    }

    std::cout << "tag = " << buffer_cache.tag[i] << ", ";
    std::cout << "counter = " << buffer_cache.counter[i] << "\n";
  }
}

