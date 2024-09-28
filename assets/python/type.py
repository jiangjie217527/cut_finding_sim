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

class Node:
    def __init__(self,depth,parent,start,count_leafs,count_merged,start_children,count_children):
        self.depth = depth
        self.parent = parent
        self.start = start
        self.count_leafs = count_leafs
        self.count_merged = count_merged
        self.start_children = start_children
        self.count_children = count_children

class Box:
    def __init__(self,min,max):
        self.min = min
        self.max = max