from os import read
from unittest import skip
from memory import *
import numpy as np
from type import *
from enum import Enum
import math
import struct

# stage: load task -> ... -> end task

class orin_data:

    def read_data(self,file_path):
        with open(file_path, 'rb') as infile:
            # Read reorder_indices
            reorder_size = struct.unpack('Q', infile.read(8))[0]  # size_t is typically 8 bytes
            reorder_indices = struct.unpack(f'{reorder_size}i', infile.read(reorder_size * 4))  # int is typically 4 bytes

            # Read tasks
            tasks_size = struct.unpack('Q', infile.read(8))[0]
            tasks = []
            for _ in range(tasks_size):
                depth = struct.unpack('i', infile.read(4))[0]
                start_node = struct.unpack('i', infile.read(4))[0]
                size = struct.unpack('i', infile.read(4))[0]
                
                leaves_size = struct.unpack('i', infile.read(4))[0]
                leaves = struct.unpack(f'{leaves_size}i', infile.read(leaves_size * 4))
                
                leaf_tasks_size = struct.unpack('i', infile.read(4))[0]
                leaf_tasks = struct.unpack(f'{leaf_tasks_size}i', infile.read(leaf_tasks_size * 4))
                
                task = Task(depth, start_node, size, leaves, leaf_tasks)
                tasks.append(task)

        return reorder_indices, tasks
    
    def __init__(self, node_num, target_size, nodes, boxes, viewpoint, render_indices, parent_indices, view_matrix, proj_matrix):
        self.reorder_indices, self.task = self.read_data("reorder.bin")
        self.nodes = nodes
        self.boxes = boxes
        self.viewpoint = viewpoint
        self.render_indices = render_indices
        self.parent_indices = parent_indices
        self.view_matrix = view_matrix
        self.proj_matrix = proj_matrix
        self.node_num = node_num
        
        self.task_size = 0
        # the address of data in v-dram is index * data_size
        # the address of data in v-buffer is index * data_size % buffer_size
        # each array correspond to a buffer, so each data array address starts from 0.
        # if read None, it means not read
        # read_list order: task buffer,nodes boffer boxes buffer



class stage:
    def __init__(self,pe_idx,node_idx = 0,stage_name = "",nxt = None,compute_cycle = 0,read_list = None,write_list = None):
        self.pe_idx = pe_idx
        self.node_idx = node_idx
        self.stage_name = stage_name
        self.nxt = nxt
        self.step = 0 # 1 means wait read 2 means compute computing 3 means writing back
        self.compute_cycle = compute_cycle
        self.read_list = read_list # read list means data location to read. the order is :task node box
        self.write_list = write_list
        self.box = None
        self.viewpoint = []
        self.target_size = 0
        self.subtree_size = 0
        self.count = 0
        self.node = None
        self.task = None
        self.result_stack = [] # stack to store the result computed the stage before
        self.reg_stack = [] # store some data in register
        self.buffer_pointer = 0
        self.num_pointer = 0

    def get_nxt(self):
        node_idx = node_idx
        if self.stage_name is "start":
            inbox = self.result_stack[0]
            buffer_size = 1024
            if (node_idx + 1) * 7 % buffer_size > (node_idx) * 7 % buffer_size:
                self.nxt = stage(self.pe_idx,node_idx,"compute_size",None,compute_cycle = 26 if inbox else 2,read_list = [[None],[None],list(range(node_idx * 7 % buffer_size,(node_idx + 1) * 7 % buffer_size))],write_list = None) # box size is 7
            else:
                self.nxt = stage(self.pe_idx,node_idx,"compute_size",None,compute_cycle = 26 if inbox else 2,read_list = [[None],[None],list(range(0,(node_idx + 1) * 7 % buffer_size)) + list(range(node_idx * 7 % buffer_size,buffer_size))],write_list = None) # box size is 7
        if self.stage_name is "compute_size":
            size = self.result_stack[0]
            if size >= self.target_size:
                self.nxt = stage(self.pe_idx,node_idx,"write_back",None,compute_cycle = 1,read_list = None,write_list = [node_idx])
            else:
                self.nxt = stage(self.pe_idx,node_idx,"write_back",None,compute_cycle = 1,read_list = None,write_list = [None])
        if self.stage_name is "write_back":
            if self.count == 0:
                self.nxt = stage(self.pe_idx,node_idx,"update_idx",None,compute_cycle = 2,read_list = [[1],[(node_idx * node_idx  + 6 )% buffer_size],[None]],write_list = None)
            else:
                self.nxt = stage(self.pe_idx,node_idx,"update_idx",None,compute_cycle = 2,read_list = [[1],[None],[None]],write_list = None)
        if self.stage_name is "update_idx":
            res = self.result_stack[0]
            if res: # have task remained is true, go to children or other node
                self.nxt = stage(self.pe_idx,node_idx,"start",None,compute_cycle = 0,read_list = [[None],[None],[None]],write_list = None)
            else: # no task-- gen new task and end
                self.nxt = stage(self.pe_idx,node_idx,"commit",None,compute_cycle = 0,read_list = [[3],[None],[None]],write_list = None)

        if self.stage_name is "commit":
            return False

        return True

    def compute(self): # change status of registers and write result value(tmp value)
        if self.stage_name is "start":
            inbox = True
            for i in range(3):
                if not (self.box.min[i] <= self.viewpoint[i] <= self.box.max[i]):
                    inbox = False
                    break
            self.result_stack[0] = inbox

        if self.stage_name is "compute_size":
            if not self.result_stack[0]:
                self.result_stack[0] = 10000000000
            else:
                p = 0
                for i in range(3):
                    p += (max(self.box_min[i],min(self.box_max[i], self.viewpoint[i])) - self.viewpoint[i]) ** 2
                self.result_stack[0] = self.box.min[3]  / math.sqrt(p)
        if self.stage_name is "write_back":
            size = self.result_stack[0]
            if size >= self.target_size:
                self.count = 1
        if self.stage.name is "update_idx":
            if self.count == 0:
                self.idx += self.subtree_size
            else:
                self.idx += 1
            if self.idx - self.task.start_node > self.task.size:
                self.result_stack[0] = False
            else:
                self.result_stack[0] = True


class PE:
    def __init__(self,idx,memory):
        self.idx = idx
        self.memory = memory       # consists of node buffer/box buffer/task buffer
        self.busy = False
        self.stage = stage(0,0,"start",None,0,None,None)
        self.memory_wait_cycle = 0 # waiting for memory to fetch data
        self.current = False       # whther we are fetching data now
        self.task = None
        
    # todo, load data for three types
    # return (bool, data_type): whether the data is ready, and the data itself
    def load_task(self,task_queue):
        task = task_queue.get()
        return (True,task)

    def load_node(self,nodes):
        pe_nodes = []
        for i in range(self.task.start_node,self.task.start_node + self.task.size):
            pe_nodes.append(Node(nodes[i][0].item(),nodes[i][1].item(),nodes[i][2].item(),nodes[i][3].item(),nodes[i][4].item(),nodes[i][5].item(),nodes[i][6].item(),nodes[i][7].item()))
        return (True,pe_nodes)

    def load_box(self,boxes):
        pe_boxes = []
        for i in range(self.task.start_node,self.task.start_node + self.task.size):
            min = []
            max = []
            for j in range(4):
                min.append(boxes[i][0][j].item())
                max.append(boxes[i][1][j].item())
            pe_boxes.append(Box(min,max))
        return (True,pe_boxes)

    def load_register_data(self,target_size,viewpoint):
        self.stage.target_size = target_size
        for i in range(3):
            self.stage.viewpoint.append(viewpoint[i].item())
        self.stage.node_idx = self.task.start_node

    
    def work(self):
        if self.stage.step == 1:
            if self.stage.buffer_pointer is 3:
                self.stage.step = 2
                self.stage.compute()
                self.stage.buffer_pointer = 0
                self.stage.num_pointer = 0
            elif self.stage.num_pointer == len(self.stage.read_list[self.stage.buffer_pointer]):
                self.stage.buffer_pointer += 1
                self.stage.num_pointer = 0
            elif self.memory_wait_cycle is not 0:
                self.memory_wait_cycle -= 1
            elif self.current:
                self.stage.num_pointer += 1
                self.current = False
            else:
                res = self.memory[self.stage.buffer_pointer].read_acq(self.stage.read_list[self.stage.num_pointer])
                if res:
                    self.current = True
                    self.memory_wait_cycle = self.memory[self.stage.buffer_pointer].read_latency

        if self.stage.step is 2:
            if self.stage.compute_cycle is 0:
                self.stage.step = 3
            else:
                self.stage.compute_cycle -= 1

        if self.stage.step == 3:
            if self.stage.buffer_pointer is 3:
                res = self.stage.get_nxt()
                if not res:
                    self.busy = False
                else:
                    self.stage = self.stage.nxt
                self.stage.num_pointer = 0
                self.stage.buffer_pointer = 0
            elif self.stage.num_pointer == len(self.stage.write_list[self.stage.buffer_pointer]):
                self.stage.buffer_pointer += 1
                self.stage.num_pointer = 0
            elif self.memory_wait_cycle is not 0:
                self.memory_wait_cycle -= 1
            elif self.current:
                self.stage.num_pointer += 1
                self.current = False
            else:
                res = self.memory[self.stage.buffer_pointer].write_acq(self.stage.write_list[self.stage.num_pointer])
                if res:
                    self.current = True
                    self.memory_wait_cycle = self.memory[self.stage.buffer_pointer].write_latency

        
    def update(self):
        if self.busy:
            self.work()
