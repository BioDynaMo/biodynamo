#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np
import sys
from math import *
import json

with open("results.json", "r") as read_file:
    data = json.load(read_file)

a = "SomaClustering1"
cpu = [0]*7
i = 0
j = 0
while i != 27:
    b = data["benchmarks"][i]["name"]
    print(b[:15])
    if a[:15] == b[:15]:
        plt.title("Soma_clustering1")
        cpu[j] = data["benchmarks"][i]["cpu_time"]
        j+=1
    i+=1
plt.plot(range(7), cpu, 'bo-')
print(cpu)
plt.show()