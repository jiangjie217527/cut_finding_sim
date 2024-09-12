from memory import *
import numpy as np
from type import *
from enum import Enum
import math

# stage: load task -> ... -> end task

class orin_data:
    def __init__(self):
        self.task = []
        self.n = 0 # single data are in register of PE. following target_size is the same
        self.target_size = 0
        self.nodes = []
        self.boxes = []
        self.viewpoint = [] # 3 data in total, same for each node/task so stay in register.following zdir is the same
        self.zdir = []
        self.viewmatrix = []
        self.projmatrix = []
        self.node_markers = []
        self.render_indices = []
        self.parent_indices = []
        
        self.task_size = 0
        # the address of data in v-dram is index * data_size
        # the address of data in v-buffer is index * data_size % buffer_size
        # each array correspond to a buffer, so each data array address starts from 0.
        # if read None, it means not read
        # read_list order: task buffer,nodes boffer boxes buffer

    def read_data(self,filename): # read binary
            with open(filename,'rb') as file:
                content = file.read()
            # ...



class stage:
    def __init__(self,pe_idx,stage_name = "",nxt = None,compute_cycle = 0,read_list = None,write_list = None):
        self.pe_idx = pe_idx
        self.stage_name = ""
        self.nxt = nxt
        self.step = 0 # 1 means wait read 2 means computeSize cycle computing 3 means writing back
        self.compute_cycle = compute_cycle
        self.read_list = read_list # read list means data location to read. the order is :task node box
        self.write_list = write_list
        self.box_min = []
        self.box_max = []
        self.viewpoint = []
        self.node = []
        self.task = []
        self.result_stack = [] # stack to store the result computed the stage before
        self.reg_stack = [] # store some data in register
        self.buffer_pointer = 0
        self.num_pointer = 0

    def get_nxt(self):
        idx = self.get_reg_data("idx",0)
        new_nxt = []
        if self.stage_name is "start":
            inbox = self.result_stack[0]
            global nodes_size,buffer_size
            if (node_idx + 1) * 7 % buffer_size > (node_idx) * 7 % buffer_size:
                self.nxt = stage(self.pe_idx,"compute_size",None,compute_cycle = 26 if inbox else 2,read_list = [[None],[None],list(range(node_idx * 7 % buffer_size,(node_idx + 1) * 7 % buffer_size))]),write_list = None # box size is 7
            else:
                self.nxt = stage(self.pe_idx,"compute_size",None,compute_cycle = 26 if inbox else 2,read_list = [[None],[None],list(range(0,(node_idx + 1) * 7 % buffer_size)) + list(range(node_idx * 7 % buffer_size,buffer_size))]),write_list = None # box size is 7


        if self.stage_name is "compute_size":
            new_nxt = []
            if not self.result_stack:
                print("error, no result data get at <get nxt>,which should be computed at previous stage")
                exit(0)
            choose = True
            size = self.result_stack[0]
            target_size = self.get_reg_data("target_size",0)
        
            if size >= target_size:
                self.nxt = stage(self.pe_idx,"write_back",None,compute_cycle = 1,read_list = None,read_list = [[None],[None],[None]],write_list = [idx])
            else:
                self.nxt = stage(self.pe_idx,"compare1",None,compute_cycle = 1,read_list = [[None],[self.node_idx * self.nodes_size % buffer_size],[None]]),write_list = [None]

        if self.stage_name is "compare1":
            res = self.result_stack[0]
            if res:
                self.nxt = stage(self.pe_idx,"update_and_write_back",None,compute_cycle = 1,read_list = [[None],[(self.node_idx * self.nodes_size  + 3 )% buffer_size]],[None]),write_list = [idx]
            else:
                self.nxt = stage(self.pe_idx,"write_back",None,compute_cycle = 0,read_list = None,read_list = [[None],[None],[None]],write_list = [idx])

        if self.stage_name is "write_back" or stage_name is "update_and_write_back":
            if self.get_reg_data("count",0) is 0:
                self.nxt = stage(self.pe_idx,"update_idx",None,compute_cycle = 2,read_list = [[1],[(self.node_idx * self.nodes_size  + 6 )% buffer_size],[None]]),write_list = None
            else:
                self.nxt = stage(self.pe_idx,"update_idx",None,compute_cycle = 2,read_list = [[1],[None],[None]]),write_list = None

        if self.stage_name is "update_idx":
            res = self.result_stack[0]
            if res: # have task remained is true, go to children or other node
                self.nxt = stage(self.pe_idx,"start",None,compute_cycle = 0,read_list = [[None],[None],[None]]),write_list = None
            else: # no task-- gen new task and end
                self.nxt = stage(self.pe_idx,"commit",None,compute_cycle = 0,read_list = [[3],[None],[None]]),write_list = None

        if self.stage_name is "commit":
            return False

        return True

    def compute(self): # change status of registers and write result value(tmp value)
        if self.stage_name is "start":
            inbox = True
            for i in range(3):
                if not (box_min[i] <= viewpoint[i] <= box_max[i]):
                    inbox = False
                    break
            self.result_stack[0] = inbox
        if self.stage_name is "compute_size":
            if not self.result_stack[0]:
                self.result_stack[0] = 10000000000
            else:
                p = 0
                for i in range(3):
                    p += (max(self.box_min[i],min(self.box_max[i],self.viewpoint[i])) - viewpoint[i]) ** 2
                self.result_stack[0] = box_min[3]  / math.sqrt(p)

        if self.stage_name is "compare1":
            self.result_stack[0] = (self.node[0] is not 0)
        if stage_name is "update_and_write_back":
            self.reg_stack[5] += self.node[4]
        if stage.name is "update_idx":
            if self.result_stack[0] is 0:
                self.idx += self.node[6]
            else:
                self.idx += 1
            if self.idx - self.task[1] > 32:
                self.result_stack[0] = False
            else:
                self.result_stack[0] = True
            
    def read(self):
        if self.buffer_pointer is 3:
            self.step = 2
            self.buffer_pointer = 0
            self.num_pointer = 0
        else if self.num_pointer == len(self.read_list[self.buffer_pointer]):
            self.buffer_pointer += 1
            self.num_pointer = 0
        else if self



    def get_reg_data(self,data_name,idx1):
        if data_name is "idx": # size 1, 1 in total
            return self.reg_stack[0]
        if data_name is "viewpoint": # size 3 ,4 in total
            return self.reg_stack[idx1 + 1]
        if data_name is "target_size":# size 1, 5 in total
            return self.reg_stack[4 + idx1] 
        if data_name is "count": # size 1, 6 in total
            return self.reg_stack[5 + idx1]


class PE:
    def __init__(self,idx,memory):
        self.idx = idx
        self.memory = memory
        self.busy = False
        self.stage = stage("start",None,0,None,None)
        self.memory_wait_cycle = 0
        self.current = False
        
    def load_data(self):
            
    
    def work(self):
        if self.stage.step == 1:
            if self.stage.buffer_pointer is 3:
                self.stage.step = 2
                self.stage.compute()
                self.stage.buffer_pointer = 0
                self.stage.num_pointer = 0
            else if self.stage.num_pointer == len(self.stage.read_list[self.stage.buffer_pointer]):
                self.stage.buffer_pointer += 1
                self.stage.num_pointer = 0
            else if self.memory_wait_cycle is not 0:
                self.memory_wait_cycle -= 1
            else if self.current:
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
            else if self.stage.num_pointer == len(self.stage.write_list[self.stage.buffer_pointer]):
                self.stage.buffer_pointer += 1
                self.stage.num_pointer = 0
            else if self.memory_wait_cycle is not 0:
                self.memory_wait_cycle -= 1
            else if self.current:
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
