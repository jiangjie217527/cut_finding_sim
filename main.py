from memory import SharedMem
from clock import Clock
from pe import PE
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

def start(tasks,n,targte_size,nodes,boxes,viewpoint,zdir,
          render_indices,node_markers,parent_indices,nodes_for_render_indices):
    cycle = 0
    task_queue = Queue()
    # init the pe status
    for i in pe_num:
        pe_list[i].input_task(tasks[i])

    while not check_finish():
        for i in pe_num:
            pe_list[i].update()
        cycle += 1

