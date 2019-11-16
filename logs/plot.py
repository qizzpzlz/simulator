import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

time =[]
value =[]
with open('performance_.txt') as f:
	#lines = f.readlines()
	for line in f.readlines():
		li = line.lstrip()
		#lin in li for lin in li.startswith("#")
		#if not (lin in li for lin in li.startswith("#")):
		if li.startswith("time"):
			time.append(line.split()[2].split(",")[0])
			value.append(line.split()[5])
			
# for x in time: 
# 	x = x.split(",")[0]
	
fig, ax = plt.subplots(2)


x = list(map(int, time))
y = list(map(int, value))

# max_value = max(y)
# max_index = y.index(max_value)
# corr_time = x[max_index]

# max_x = x[max_index-5:max_index+5]
# max_y = y[max_index-5:max_index+5]

pending_times = []
pending_counts = []
with open('pending.txt') as f:
	for line in f.readlines():
		time, count = line.split(', ')
		time = int(time)
		count = int(count)
		pending_times.append(time)
		pending_counts.append(count)

ax[0].plot(x,y)
ax[1].plot(pending_times, pending_counts)

plt.show()
fig.savefig('plot.png')
