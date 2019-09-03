import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

with open('performance_100000.txt') as f:
	lines = f.readlines()
	time = [line.split()[2].split(",")[0] for line in lines]
	value = [line.split()[5] for line in lines]

# for x in time: 
# 	x = x.split(",")[0]
	
fig, ax = plt.subplots()


# print(time)
# print(value)

x = list(map(int, time))
y = list(map(int, value))

ax.plot(x,y)

#plt.scatter(x, y, s=3)

ymax = max(y)
xpos = y.index(ymax)
xmax = x[xpos]

text= "x={:.3f}, y={:.3f}".format(xmax, ymax)
bbox_props = dict(boxstyle="square, pad=0.4", ec="k", lw=1, fc="yellow")
arrowprops=dict(arrowstyle="->", color= 'red', lw= 2)
kw = dict(xycoords='data',textcoords="axes fraction",
          arrowprops=arrowprops, bbox=bbox_props, ha="right", va="center")
ax.annotate(text, xy=(xmax, ymax), xytext=(0.6,1), **kw)


#print(data)
plt.show()
fig.savefig('plot_OLB_100000.png')