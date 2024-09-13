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
        - if **smaller or the leaf of the hierarchy**: `cur_id += 1`, append `cur_id` to the array of **selected cut**
5. after the loop, write the **selected cut/leaves** back to the **commit buffer** 

In brief, we can break the process of step 4 (dealing with single node) into **three stages**
- fetching nodes/box data
- compute size
- compare and commit

### 2.3 Commit Buffer

commit buffer contains two arrays, one filled with `cut_node`, and the other with the pair `<task_id, leaf_node>`

### 2.4 Scheduler

Scheduler deals with these two arrays in parellel

- as for `cut_nodes`, check for incoming nodes and put them in result
- as for `leaf_nodes`
    - call the data cache to replace `task_id` with the id that this `leaf_node` would invoke
    - put these new ids into `task_queue`

## Discussion of details

