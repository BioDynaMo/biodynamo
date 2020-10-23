#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np
import sys
from math import *
import json


def data_cpu(name_demo):
    nb = len(name_demo)
    a = name_demo
    cpu = [0]*7
    i = 0
    j = 0
    while i != 27:
        b = data["benchmarks"][i]["name"]
        if a[:nb] == b[:nb]:
            cpu[j] = data["benchmarks"][i]["cpu_time"]
            j+=1
        i+=1
    return cpu

def graph(name_demo):
    cpu = data_cpu(name_demo)
    moy = 0
    h = 0
    tmp = 0
    while h < 7:
        tmp = cpu[h] + tmp
        h += 1
    moy = tmp / 7
#    plt.plot(range(7), cpu, 'bo-')
    fig, ax = plt.subplots()
 #   plt.plot(range(1), moy, 'bo-')
    xlabels = ['str']
    ax.plot([1], moy, 'bo-')
    ax.set_title(name_demo)
    ax.set_xticks([1])
    ax.set_xticklabels(xlabels, rotation=40)
    plt.show()
    return

with open("results.json", "r") as read_file:
    data = json.load(read_file)

graph("SomaClustering1")
#graph("SomaClustering0")
#graph("TumorConcept1")
#graph("TumorConcept0")
