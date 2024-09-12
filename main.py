from itertools import count
from memory import SharedMem
from clock import Clock
from pe import PE, orin_data
from queue import Queue

pe_list = []
pe_num = 4
for i in range(4):
    pe_list.append(PE(i))

def check_finish():
    for i in pe_num:
        if pe_list[i].busy == True:
            return False
    return True

# first row is meta data and second row is outout buffer pointer
# tasks: all possible tasks in an finite array

def start(node_num, target_size, nodes, boxes, viewpoint, render_indices, parent_indices, view_matrix, proj_matrix):
    cycle = 0
    count = 0
    task_queue = Queue()
    for i in range(pe_num) :
        task_queue.put(i)

    OrinData = orin_data(node_num=node_num, target_size=target_size, nodes=nodes, boxes=boxes, viewpoint=viewpoint, render_indices=render_indices, parent_indices=parent_indices, view_matrix=view_matrix, proj_matrix=proj_matrix)

    while not check_finish():
        for i in pe_num:
            pe_list[i].update()
        cycle += 1

    return count

def reboot():
    # init the shared memory
    shared_mem = SharedMem()
    # init the clock
    clock = Clock()
    for i in range(4):
        pe_list.append(PE(i))
    