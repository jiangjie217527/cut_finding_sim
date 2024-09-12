import numpy as np

class Task:
    def __init__(self,depth=0,start_node=0,size=0,leaves_size=0,leaves=[],leaf_tasks_size=0,leaf_tasks=[]):
        self.depth = depth
        self.start_node = start_node
        self.size = size
        self.leaves_size = leaves_size
        self.leaves = leaves
        self.leaf_tasks_size = leaf_tasks_size
        self.leaf_tasks = leaf_tasks

    def format_into_data(self):
        data = np.zeros([5 + self.leaves_size + self.leaf_tasks_size,1],dtype = int)
        data[0] = self.depth
        data[1] = self.start_node
        data[2] = self.size
        data[3] = self.leaves_size
        for i in range(4,self.leaves_size + 4):
            data[i] = self.leaves[i - 4]
        data[self.leaves_size + 4] = self.leaf_tasks_size
        for i in range(self.leaves_size + 5,5 + self.leaves_size + self.leaf_tasks_size):
            data[i] = self.leaf_tasks[i - 5 - self.leaves_size]
        return data

class Node:
    def __init__(self,depth,parent,start,count_leafs,count_merged,start_children,count_children):
        self.depth = depth
        self.parent = parent
        self.start = start
        self.count_leafs = count_leafs
        self.count_merged = count_merged
        self.start_children = start_children
        self.count_children = count_children

    def format_into_data(self):
        data = np.zeros([7,1],dtype=int)
        data[0] = self.depth
        data[1] = self.parent
        data[2] = self.start
        data[3] = self.count_leafs
        data[4] = self.count_merged
        data[5] = self.start_children
        data[5] = self.count_children
