from itertools import count
from memory import Buffer
from clock import Clock
from pe import PE, orin_data
from queue import Queue

pe_list = []
pe_num = 4
buffer_num = 3
memory = []
def check_finish():
    for i in pe_num:
        if pe_list[i].busy == True:
            return False
    return True

# first row is meta data and second row is outout buffer pointer
# tasks: all possible tasks in an finite array

def start(node_num, target_size, nodes, boxes, viewpoint, render_indices, parent_indices, view_matrix, proj_matrix):
    cycle = 0
    task_queue = Queue()
    for i in range(pe_num):
        task_queue.put(i)
        pe_list[i].busy = True

    OrinData = orin_data(node_num=node_num, target_size=target_size, nodes=nodes, boxes=boxes, viewpoint=viewpoint, render_indices=render_indices, parent_indices=parent_indices, view_matrix=view_matrix, proj_matrix=proj_matrix)

    while not check_finish():
        for i in range(pe_num):
            if not pe_list[i].data_load:
                pe_list[i].memory_wait_cycle = 100
                pe_list[i].data_load = True
                pe_list[i].load_task(task_queue,OrinData.task)
                pe_list[i].load_nodes(nodes)
                pe_list[i].load_box(boxes)
                pe_list[i].load_register_data(target_size,viewpoint)
            else:
                pe_list[i].update()
        cycle += 1
        for i in range(buffer_num):
            memory[i].memory_return_check()

    # return the size of render_indices
    return render_indices.size

def reboot():
    # init the shared memory
    nodes_buffer = Buffer(10,10,256 * 1024)
    box_buffer = Buffer(10,10,256 * 1024)
    task_buffer = Buffer(10,10,256 * 1024 * 2)
    memory.append(task_buffer)
    memory.append(nodes_buffer)
    memory.append(box_buffer)
    # init the clock
    clock = Clock()
    for i in range(4):
        pe_list.append(PE(i,memory))
    

if __name__ == "__main__":
    reboot()
    start()