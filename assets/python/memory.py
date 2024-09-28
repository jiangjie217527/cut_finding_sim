from clock import Clock 
import numpy as np
from queue import Queue
# 16 banks

class Buffer:
    def __init__(self,read_latency,write_latency,total_size):
        self.bank_num = 16
        self.bank_used = np.zeros([self.bank_num,1],dtype=np.int16)
        self.bank_clock = np.zeros([self.bank_num,1],dtype = int)
        self.total_size = total_size # e.g. 256 * 1024 ( * 4B = 1MB)
        # self.mem = np.zeros([self.total_size,1],dtype=int) 
        # self.mem = []
        # for i in range(self.bank_num):
        #     self.mem.append([])
        # self.mem_used = 0
        self.read_latency = read_latency
        self.write_latency = write_latency

    def read_acq(self,addr):
        bank_idx = addr % self.bank_num
        if self.bank_used[bank_idx] != 0:
            return False
        self.bank_used[bank_idx] = 1
        self.bank_clock[bank_idx] = self.read_latency
        # if addr < self.mem_used:
        #     self.bank_return_cache[bank_idx,1] = self.mem[addr / self.bank_num][bank_idx]
        return True

    def write_acq(self,addr):
        bank_idx = addr % self.bank_num
        if self.bank_used[bank_idx] != 0:
            return False
        self.bank_used[bank_idx] = 2
        self.bank_clock[bank_idx] = self.write_latency
        return True
    
    def memory_return_check(self):
        for i in range(self.bank_num):
            self.bank_clock[i] = max(self.bank_clock[i] - 1,0)
            if self.bank_clock[i] == 0:
                # if self.bank_used[i] == 1: # read acquire
                #     self.bank_return_cache[i,0] = 1
                self.bank_used[i] = 0