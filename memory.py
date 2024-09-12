from clock import Clock 
import numpy as np
from queue import Queue
from task import Task
# 16 banks

class Buffer:
    def __init__(self,read_latency,write_latency):
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

    def write_acq(self,addr,value):
        bank_idx = addr % self.bank_num
        if self.bank_used[bank_idx] != 0:
            return False
        self.bank_used[bank_idx] = 2
        self.bank_clock[bank_idx] = self.write_latency
        return True
    
    # def memory_return_check(self):
    #     for i in range(self.bank_num):
    #         self.bank_clock[i] = max(self.bank_clock[i] - 1,0)
    #         if self.bank_clock[i] == 0:
    #             # if self.bank_used[i] == 1: # read acquire
    #             #     self.bank_return_cache[i,0] = 1
    #             self.bank_used[i] = 0
                
    # def load_data(self,data,length):
    #     start_ptr = self.mem_used
    #     for i in range(start_ptr,start_ptr + length):
    #         self.mem[i/self.bank_num,i%self.bank_num] = data[i - start_ptr]
    #     self.mem_used += length
    #     return start_ptr

        
class SRAM:
    def __init__(self,read_latency,write_latency,shared_mem):
        # self.capacity = 256 # 256 * 4 B = 1kB, 1 Kb in total
        self.mem = np.zeros([self.capacity,2],dtype=int) # address & value pair
        self.size = 0
        self.read_latency = read_latency
        self.write_latency = write_latency
        self.shared_mem = shared_mem # quote of class
        self.acq_addr = []
        self.ret_buff = 0

    def read_acq(self,addr):
        for i in range(self.size):
            if self.mem[i,0] == addr:
                # shift the newest element
                tmp = self.mem[i]
                for j in range(i,self.size - 1):
                    self.mem[j] = self.mem[j+1]
                self.mem[self.size] = tmp
                # maintain lru structure
                return (True,tmp[1])
        # find element in shared memory
        # maybe bank conflict happen
        if self.shared_mem.read_acq(addr):
            self.acq_addr.append(addr)            
        return (False,0)


    def write(self,addr,value):
        idx = self.size
        cached = False
        for i in reversed(range(idx)):
            if self.mem[i,0] == addr:
                cached = True
                for j in range(i + 1,idx):
                    self.mem[j] = self.mem[j - 1]
                self.mem[idx] = [addr,value]
                break
        if not cached:
            if idx < self.capacity:
                self.mem[idx] = [addr,value]
                self.size += 1
            else:
                for i in range(1,idx):
                    self.mem[i - 1] =self.mem[i]
                self.mem[idx] = [addr,value]
        return True

    def write_acq(self,addr,value)
        self.write(addr,value)
        return self.shared_mem.write_acq(addr,value)

    def memory_read_check(self):
        self.shared_mem.memory_return_check()
        for i in range(self.acq_addrr):
            bank_idx = i % self.shared_mem.bank_num
            tmp = self.shared_mem.bank_return_cache[bank_idx]
            if tmp[0] == 1:
                self.write(i,tmp[1])
                self.shared_mem.bank_return_cache[bank_idx,0] = 0


class task_queue:
    def __init__(self,read_latency,write_latency):
        self.read_latency = read_latency
        self.write_latency = write_latency
        self.task_queue = Queue()
        self.task_num = 0
        self.read_busy = False
        self.write_busy = False
        self.read_clock = 0
        self.write_clock = 0
        self.read_cache = 0 # task id
        self.write_cache = 0 # task id

    def push_task(self,task):
        if self.write_busy:
            return False
        self.write_busty = True
        sefl.write_cache = task
        self.write_clock = self.write_latency
        return True

    def read_task(self):
        if self.read_busy or self.task_num == 0:
            return False
        self.read_busy = True
        self.read_clock = self.read_latency
        return True

    def update(self):
        if self.read_busy:
            self.read_clock = max(0,self.read_clock - 1)
            if self.read_clock == 0:
                self.read_cache = self.task_queue.get()
                self.read_busy = False
                self.task_num -= 1
        if self.write_busy:
            self.write_clock = max(0,self.write_clock - 1)
            if self.write_clock == 0:
                self.task_queue.push(self.write_cache)
                self.write_busy = False
                self.task_num += 1
