class Clock:
    def __init__(self):
        self.freq = 1024 # MHz
        self.time = 0.0
        self.cycle = 0

    def step(self):
        self.cycle += 1

    def forward(self,cyc_num):
        self.cycle += cyc_num

    def get_time(self):
        # return ms
        return self.cycle / self.freq / 1000000 * 1000

if __name__ == "__main__":
    clock = Clock()
    clock.step(1024)
    print(clock.get_time())
