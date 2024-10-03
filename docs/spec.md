# Specification

author: Xingyang Li

----

## 1. Data Cache

#### 1.1 Datatype and respective members, types

- Nodes: depth, subtree_size, start, parent_start, size, siblings, parent_id, count_leaf $7\times 4+1=29$ bytes
- Boxes: $8\times 4$ bytes
- Subtasks: <leaf_node, begin_leaf_task, end_leaf_task>, thus the size is at most `PE_TASK_SIZE`$\times 8 = 256$ bytes
- Tasks: start_node, task_size, $2\times 4$ bytes

So an entry in the Data Cache costs: $2216$ bytes.

## 2. Control Flow of Accelerator

### 2.1 Preload Stage

1. load `viewpoint, view_matrix, proj_matrix, target_size` to each of the PE

### 2.2 PE Unit

1. fetch the first `task_id` in `task_queue`
2. see the data cache to get corresponding entry (task)
3. set `cur_id = cur_task.start_id`
4. loop: checking each node
    - is `cur_id` in ``[start_id, start_id + task_size)``? if not, quit
    - fetch Node/Box data from the cache
    - make inbox/frustum check and size computing(projection unit) in **parellel**(like shortcut eval)
    - compare size with `target_size`
        - if **bigger and not the leaf of the hierarchy**: `cur_id += cur_node.subtree_size`, if this is a **leaf node of the subtree** (`subtree_size == 1`), append `cur_id` to the array of **selected leaves**
        - if **smaller or the leaf of the hierarchy**: `cur_id += 1`, append `cur_id` to the result array of **selected cut**
    - calculate interpolation weights and update siblings if this node is selected as a cut
5. After the loop, write the finished `task_id` to the commit buffer; In the meantime, send a signal to the scheduler to notify that after finding subtasks corresponding to the last leaf, we can invalidate the respective cache entry of such `task_id`.

The full process of dealing a node consists of about $12+6$ cycles, and can be divided to **18** stages of pipelining as there isn't any data dependency.

### 2.3 Commit Buffer

commit buffer contains one array filled with the pair `<task_id, leaf_node>`, another containing the currently done tasks, the other containing the fetched new `task_id` array.

### 2.4 Scheduler

The scheduler has to the deal with the commit buffer in this way:

- as for `leaf_nodes`
    - look in the cache to search for all the associated subtasks for this leaf node
    - put their `task_id` in the `waitlist`
    - if a task is finished, **invalidate it**
    - if there's a task finished, put the first task in the waiting queue into the cache (ask for DRAM), and fill it into `task_queue` once the DRAM has finished.

## 3. Discussion of details

### 3.1 How to decide whether our simulator has halted

The simulator halts if and only when:

- every PE doesn't have a task to work with
- the scheduler have nothing to schedule
- there's no entry busy/valid in DCache

And it can be shown that none of the aforementioned conditions can be wiped out.

### 3.2 Buffer Cache of our DCache

We use a buffer to serve as an additional storage for upcoming tasks, so that they don't need to wait until there bank has a vacancy, and it successfully handles the case where the remainder after `BankNum` **is biased**.

#### 4. Configurations and Memory Footprint

The `common.hpp` is listed as follows.

```c++
#ifndef HGS_COMMON_HPP_
#define HGS_COMMON_HPP_

#include <cassert>
#include <iostream>

#include "half.hpp"
#include "types.hpp"

constexpr int PipelineStage = 18;
constexpr int CutSelectStage = 12;
constexpr int WeightCalcStage = PipelineStage - CutSelectStage;
constexpr int DRAMWordsPerCycle = 4;
constexpr int TaskQueueSize = 12;
constexpr int PENum = 4;

// for DCache
constexpr int BankNum = 8;
constexpr int BankSize = 4 * (TaskQueueSize + PENum) / BankNum;
constexpr int PortWordsPerCycle = 4;
constexpr int PortNum = 4;
constexpr int CacheWordsPerCycle = PortWordsPerCycle * PortNum;
constexpr int BufferCacheSize = 8;
// for Task
constexpr int MaxTaskSize = 32;
constexpr int MaxSubtaskSize = 32;
constexpr int SizeOfTask = sizeof(int) + sizeof(int) + sizeof(Box) + sizeof(int);
constexpr int SizeofBox = sizeof(Point4) + sizeof(Point4);
constexpr int SizeOfNode = sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(float) + sizeof(int) + sizeof(bool);
constexpr int SizeOfSubtask = MaxSubtaskSize * (sizeof(int) + sizeof(int) + sizeof(int));
constexpr int SizeOfCacheData = SizeOfTask + SizeOfSubtask + MaxTaskSize * SizeOfNode + MaxTaskSize * SizeofBox;
// for Scheduler
constexpr int MaxLeafBufferSize = 32;
#endif // HGS_COMMON_HPP_
```
So the size of our DataCache is $160$ KB.
