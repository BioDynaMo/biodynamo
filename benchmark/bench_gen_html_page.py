#!/usr/bin/python3

import sys
import os
import glob

file = glob.glob("benchmark/*.png")
nb = len(file)
i = 0
my_file = open("benchmark/my_web.html", "w")
my_file.write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n")
my_file.write("<html>\n")
my_file.write("<head>\n")
my_file.write("<meta charset=\"utf-8\">\n")
my_file.write("<title>my_web</title>\n")
my_file.write("</head>\n")
my_file.write("<body>\n")
my_file.write("<h1>Benchmark</h1>\n")
my_file.write("<p>One of our goals is to produce highly efficient code  as from CPU, also memory usage.\n")
my_file.write("Because of that we constantly evaluate the quality of the biodynamo code. Our <a href=\"https://en.wikipedia.org/wiki/Benchmark_(computing)\">benchmark</a> code evauates biodynamo library based on number of demo use-cases, from the CPU and memory usage.\n")
my_file.write("<h2>CPU time and memory</h2>\n")
my_file.write("<p>To evaluate the performance of BioDynaMo, we use the benchmark for each demo of the biodynamo. We ran the demos Soma Clustering and Tumor Concepta and compare the values. In the graphics, the bule plots is for the CPU Time and the red plots is for the Memory Usage. The x axis is for the different version of biodynamo.<p>\n")
my_file.write("<div class=\"figure\">\n")
while i != nb:
    my_file.write("<img src=\""+file[i][10:]+"\"\n")
    my_file.write("alt=\""+file[i][10:]+"\"\n")
    my_file.write("title=\""+file[i][10:]+"\">\n")
    i+=1

my_file.write("</div>\n")
my_file.write("</body>\n")
my_file.write("</html>\n")
my_file.close()