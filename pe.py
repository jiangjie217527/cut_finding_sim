from memory import *
import numpy as np
from type import *
from enum import Enum

class PE_status(Enum):
    empty = 0
    wait_memory = 1
    bank_conflict = 2
    compute = 3

class PE_step(Enum):
    start = 0
    load_node = 1
    compute_size_GPU = 2
    compare = 3
    write_back = 4
    inbox = 5

class inbox_ins(Enum):


class PE:
    def __init__(self,idx,memory):
        self.idx = idx
        self.memory = memory
        self.busy = False
        self.status = PE_status.empty
        self.wait_memory_access_cycle = 0
        self.wait_compute_cycle = 0
        
        self.task = Task()

        self.step = PE_step.start

    def input_task(self,task):
        self.busy = True
        self.task = task
        self.step = PE_step.load_node

    def work(self):
        work_end = False
        
        for _,step in PE_step.__members__.items():
            if self.step == PE_step.inbox:

            if self.step == PE_step.compute_size_GPU:

            if self.step == PE_step.compare:

            if self.step == PE_step.write_back:


        if work_end:
            self.busy = False

        
    def update(self):
        if self.busy:
            self.work()
