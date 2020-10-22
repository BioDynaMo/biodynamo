#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np
import sys
from math import *

# To find the number of tests in the Demo
def nb_data():
    runtime = open("runtime.txt")
    i = 0
    while 1:
        run = runtime.readline()
        if run == "###################################\n":
            break
        i += 1
    runtime.close()
    return i - 2

# To take all the data of the Demo
def take_data_of_the_demo(runtime, x):
    i = 0
    data_run = [0] * x
    while 1:
        run = runtime.readline()
        if run == "###################################\n":
            break
        data_run[i] = run.strip()
        i += 1
    return data_run, i

def take_x(runtime):
    i = 0
    while 1:
        run = runtime.readline()
        if run == "###################################\n":
            break
        i += 1
    return i

def search_loop(i, x):
    if sys.argv[2] == "soma_clustering0":
        av = "soma_clustering: export=false"
    if sys.argv[2] == "soma_clustering1":
        av = "soma_clustering: export=true"
    if sys.argv[2] == "tumor_concept0":
        av = "tumor_concept: export=false"
    if sys.argv[2] == "tumor_concept1":
        av = "tumor_concept: export=true"
    demo = av
    runtime = open("runtime.txt")
    j = 0
    while j <= i:
        data = runtime.readline()
        if j == i and data == demo+'\n':
            break
        if data == demo+'\n':
            j += 1
    data = runtime.readline()
    x = take_x(runtime)
    runtime.close()
    return x

# Take the Data Demo
def take_demo_time(i, x):
    if sys.argv[2] == "soma_clustering0":
        av = "soma_clustering: export=false"
    if sys.argv[2] == "soma_clustering1":
        av = "soma_clustering: export=true"
    if sys.argv[2] == "tumor_concept0":
        av = "tumor_concept: export=false"
    if sys.argv[2] == "tumor_concept1":
        av = "tumor_concept: export=true"
    demo = av
    x = search_loop(i, x)
    runtime = open("runtime.txt")
    j = 0
    while j <= i:
        data = runtime.readline()
        if j == i and data == demo+'\n':
            break
        if data == demo+'\n':
            j += 1
    data = runtime.readline()
    data_run, h = take_data_of_the_demo(runtime, x)
    runtime.close()
    return data_run, x

# Take the CPU in the Data Demo
def take_data_CPU(data_run, x):
    data_CPU = [0] * x
    i = 0
    while i < x:
        CPU = data_run[i].split()
        data_CPU[i] = int(CPU[2]) * 10**(-9)
        i += 1
    return data_CPU

# How many times we used the script "my_bensh.sh"
def nb_runtimes():
    if sys.argv[2] == "soma_clustering0":
        av = "soma_clustering: export=false"
    if sys.argv[2] == "soma_clustering1":
        av = "soma_clustering: export=true"
    if sys.argv[2] == "tumor_concept0":
        av = "tumor_concept: export=false"
    if sys.argv[2] == "tumor_concept1":
        av = "tumor_concept: export=true"
    demo = av
    runtime = open("runtime.txt")
    data = runtime.readline()
    i = 0
    while 1:
        if data == demo+'\n':
            runtime.readline()
            i += 1
        if data == "":
            if i == 0:
                print("Choose a Demo:\nsoma_clustering\ntumor_concept")
                exit()
            return i
        data = runtime.readline()
    return i

# Graph to directly compare Data
def compare(i):
    j = 0
    x = nb_data()
    while j < i:
        data_run, x = take_demo_time(j, x)
        data_CPU = take_data_CPU(data_run, x)
        plt.plot(range(x), data_CPU, 'bo-')
        j += 1
    return

def errorbar_data(data_CPU, moy, j):
    i = 0
    tmp = 0
    while i < j:
        tmp = (data_CPU[i] - moy) ** 2 + tmp
        i += 1
    tmp = sqrt(tmp/j)
    return tmp

# Graph to compare the average Data
def average(i):
    j = 0
    moy = [0] * i
    error_data = [0] * i
    x = nb_data()
    while j < i:
        tmp = 0
        h = 0
        data_run, x = take_demo_time(j, x)
        data_CPU = take_data_CPU(data_run, x)
        while h < x:
            tmp = data_CPU[h] + tmp
            h += 1
        moy[j] = tmp / x
        error_data[j] = errorbar_data(data_CPU, moy[j], h)
        j += 1
    plt.errorbar(range(i), moy, error_data, fmt='bo-', ecolor= 'r')
    return

# main program
def main():
    i = nb_runtimes()
    x = 1
    y = 0
    try:
        if sys.argv[1] == "--compare" or sys.argv[1] == "-c":
            compare(i)
        if sys.argv[1] == "--average" or sys.argv[1] == "-a":
            average(i)
    except:
        print("Error")
    plt.xlabel(sys.argv[2])
    axis = plt.axis()
    size = -0.5/(axis[2]-axis[3])/100
    plt.axis([axis[0], axis[1], axis[2]*(1-size), axis[3]*(1+size)])
    plt.show()

if __name__ == "__main__":
    try:
        main()
    except:
        print("USAGE: ./graph.py [av1] [av2]\n\nav1:\t-a or --average for average\n\t-c or --compare for compare\n\nav2:\tsoma_clustering1\n\tsoma_clustering0\n\ttumor_concept0\n\ttumor_concept1")