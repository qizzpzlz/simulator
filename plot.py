import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

with open('performance_100000.txt') as f:
	lines = f.readlines()
	time = [line.split()[2].split(",")[0] for line in lines]
	value = [line.split()[5] for line in lines]

# for x in time: 
# 	x = x.split(",")[0]
	
fig = plt.figure(figsize=(200, 20))

print(time)
print(value)

x = list(map(int, time))
y = list(map(int, value))

plt.scatter(x, y, s=3)

#print(data)
plt.show()
fig.savefig('plot_OLB_100000.png')