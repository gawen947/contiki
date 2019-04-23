import fileinput
import numpy as np

xs=[]
for line in fileinput.input():
    value = float(line)

    xs.append(value)
ns=np.array(xs)
print(np.average(ns), np.std(ns))
