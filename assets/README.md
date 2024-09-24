# Target and Expected Output

1. PE cycles
2. memory access times
3. bank conflict times

# About the memory read and write

if PE want to read the memory, it give out a read acquire to SRAM. SRAM first find the address in its cache(the cache is organized using LRU algorighm). If it exists, return right (next cycle perhaps, wait to modify). If it don't exist, the SRAM give out a read acquire to shared memory, and wait for shared memory's return. One case is that shared memory's corresponding bank is busy, so shared memory refused the read acquire. So this acquire is not valid, the SRAM shouldn't push the address in a list and will send requires consistently. On the other hand, if shared memory accepts the require, it will check the valid output of the bank(according to the list) until read it.

# About data structure

struct node:(size 6 byte)
~~~
    int depth = -1;
	int parent = -1;
	int start;
	int count_leafs;
	int count_merged;
	int start_children;
	int count_children;
~~~

struct box:(size 8 byte)
~~~
    float xyz_min[4];
    float xyz_max[4];
~~~

struct viewport:(size 3 byte)
~~~
    float xyz[3];
~~~

struct task:(5 + v1\_size + v2\_size)
~~~
    int depth;
    int start_node;
    int size;
    int vector1_size; // 4th byte
    int leaves[vector1_size]; // maybe repeat
    int vector2_size;
    int leaf_task[vector2_size]
~~~

Each class has a function called "format into data", which format the data of the python class into data array to load into simulated memory


# About the PE

When the task **starts**, the origin calls the pe's input function. Then the pe get into bust status and update its status every clock. When the task end, it get out of busy and get next task queue.

Every PE looks like a automaton. Every clock, it is called to work and step a stage.
