#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np
import sys
from math import *
import json
import os
import csv


def data_cpu(name_demo, iteration, ii):
    cpu = [0]*iteration
    i = 0
    j = 0
    a = name_demo
    nb = len(name_demo)
    while i != iteration*ii:
        b = data["benchmarks"][i]["name"]
        if a[:nb] == b[:nb]:
            cpu[j] = data["benchmarks"][i]["cpu_time"]
            j+=1
        i+=1
    return cpu

def graph(name_demo, iteration, i):
    cpu = data_cpu(name_demo, iteration, i)
    moy = 0
    h = 0
    tmp = 0
    while h < iteration:
        tmp = cpu[h] + tmp
        h += 1
    moy = tmp / iteration
    fig, ax = plt.subplots()
    xlabels = ['str']
    ax.plot([1], moy, 'bo-')
    ax.set_title(name_demo)
    ax.set_xticks([1])
    ax.set_xticklabels(xlabels, rotation=40)
#    plt.savefig(name_demo+'.')
#    plt.show()
    return name_demo, moy

def name(i):
    name_data = data["benchmarks"][i]["name"]
    z = 0
    while z in range( len(name_data)):
        if name_data[z] == '/': break
        z += 1
    name_data = name_data[:z]
    return name_data

def names():
    name_datas = [0] * len(data["benchmarks"])
    i = 0
    j = 0
    while i != len(data["benchmarks"]):
        if name(i) != name_datas[j-1]:
            name_datas[j] = name(i)
            j+=1
        i+=1
    name_datas = name_datas[:j]
    return name_datas, j

def iteration():
    tmp = [0]
    i = 0
    x = 0
    j = 0
    while i != len(data["benchmarks"]):
        tmp = name(i)
        if tmp == name(i+1):
            j+=1
            i+=1
        else:
            break
    return j+1

def store(cpu, name_demo):
    version = sys.argv[2]
    print(name_demo)
    csvfile = open('benchmark/'+name_demo+'.csv', 'a+')
    writer = csv.writer(csvfile)
    writer.writerow( (version, cpu) )
    csvfile.close()

def main():
    i = 0
    it = iteration()
    name_datas, j = names()
    cpu = [0]*j
    name_demo = [0]*j
    while i < j:
        name_demo[i], cpu[i] = graph(name_datas[i], it, j)
        store(cpu[i], name_demo[i])
        i += 1
    # print(cpu)
    # print(name_demo)
#    store('soma_clustering')
    return

if __name__ == "__main__":
   file = sys.argv[1]
   with open(file, "r") as read_file:
       data = json.load(read_file)
   try:
        main()
   except:
       print("ERROR")
#    main()