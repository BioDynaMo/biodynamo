import os
import re

def GetBinaryName():
    with open("CMakeLists.txt") as f:
        content = f.read()
        return re.search('project\((.*)\)', content).group(1)
