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
    memory = [0]*iteration
    i = 0
    j = 0
    a = name_demo
    nb = len(name_demo)
    while i != iteration*ii:
        b = data["benchmarks"][i]["name"]
        if b != b[:nb]+"/process_time":
            i+=1
            continue
        if a[:nb] == b[:nb]:
            cpu[j] = data["benchmarks"][i]["cpu_time"]
            memory = data["benchmarks"][i]["memory"]
            j+=1
        i+=1
    return cpu, memory

def graph(name_demo, iteration, i):
    cpu, memory = data_cpu(name_demo, iteration, i)
    moy = 0
    h = 0
    tmp = 0
    if iteration != 1:
        iteration-=3
    while h < iteration:
        tmp = cpu[h] + tmp
        h += 1
    moy = tmp / iteration
    return name_demo, moy, memory

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

def store(cpu, name_demo, memory):
    version = sys.argv[2]
    csvfile = open('benchmark/'+name_demo+'.csv', 'a+')
    writer = csv.writer(csvfile)
    writer.writerow( (version, cpu, memory) )
    csvfile.close()

def nb_data(name_demo):
    with open('benchmark/'+name_demo+'.csv') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        line_count = 0
        for row in csv_reader:
            line_count += 1
    return line_count


def plot(name_demo):
    nb = nb_data(name_demo)
    v = [0]*nb
    value = [0]*nb
    value2 = [0]*nb
    fig, ax = plt.subplots(2, sharex='col', sharey='row')
    with open('benchmark/'+name_demo+'.csv') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        line_count = 0
        for row in csv_reader:
            v[line_count] = row[0]
            value[line_count] = float(row[1])/10**9
            value2[line_count] = float(row[2])
            line_count += 1
    xlabels = v
    ax[0].plot(range(nb), value, 'bo-')
    ax[1].plot(range(nb), value2, 'ro-')
    ax[1].set_title(name_demo)
    ax[1].set_xticks(range(nb))
    ax[1].set_xticklabels(xlabels, rotation=10)
    plt.savefig('benchmark/'+name_demo+'.png')
    return

def main():
    i = 0
    it = iteration()
    name_datas, j = names()
    cpu = [0]*j
    name_demo = [0]*j
    memory = [10]*j
    while i < j:
        name_demo[i], cpu[i], memory[i] = graph(name_datas[i], it, j)
        store(cpu[i], name_demo[i], memory[i])
        plot(name_demo[i])
        i += 1
    return

if __name__ == "__main__":
    file = sys.argv[1]
    with open(file) as read_file:
        data = json.load(read_file)
        read_file.close()
#    try:
    main()
#    except:
#        print("ERROR")
