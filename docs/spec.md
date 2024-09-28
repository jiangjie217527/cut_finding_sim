# Specification

## 1. Data Cache

#### 1.1 Datatype and respective members, types

Nodes: depth, parent, subtree_size, $3\times 4$ bytes
Boxes: $8\times 4$ bytes
Subtasks: <leaf_node, leaf_task>, **size unclear!!**
Tasks: start_node, task_size, $2\times 4$ bytes

Index: task_id, $4$ bytes

#### 1.2 

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
5. after the loop, write the finished `task_id` to the commit buffer

The full process of dealing a node consists of about 12 cycles, and can be divided to **no less than 10** stages of pipelining.

### 2.3 Commit Buffer

commit buffer contains one array filled with the pair `<task_id, leaf_node>`; and another containing the currently done tasks

### 2.4 Scheduler

The scheduler has to the deal with the commit buffer in this way:

- as for `leaf_nodes`
    - look in the cache to search for all the associated subtasks for this leaf node
    - put their `task_id` in the `waitlist`
    - if a task is finished, **invalidate it**
    - if there's a task finished, put the first task in the waiting queue into the cache (ask for DRAM), and fill it into `task_queue` once the DRAM has finished.

## 3. Discussion of details

### 3.1 How to decide whether our simulator has halted



