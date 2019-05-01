#!/usr/bin/env python3
# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

import os
import sys
import subprocess as sp

DIR_PATH = os.path.dirname(os.path.realpath(__file__))
LINK_DEF_FILE = '{}/gui_LinkDef.h'.format(DIR_PATH)
DICT_FILE = '{}/gui_Dict.cc'.format(DIR_PATH)
HEADER_FILE = '{}/gui.h'.format(DIR_PATH)
PCM_FILE = '{}/gui_Dict_rdict.pcm'.format(DIR_PATH)

def generateLinkDef():
    cmd = '''awk '/^class .* {/{print "#pragma link C++ class " $2 ";"}' '''
    cmd = cmd + HEADER_FILE
        
    f = os.popen(cmd)
    output = f.read()
    f.close()

    f = open(LINK_DEF_FILE, "w")
    f.write('''#ifdef __CLING__\n\n''' +
            '''#pragma link off all globals;\n''' +
            '''#pragma link off all classes;\n''' +
            '''#pragma link off all functions;\n\n''' + 
            output + 
            '''\n#endif\n''')
    f.close()

def cleanFiles():
    try:
        os.remove(LINK_DEF_FILE)
        os.remove(DICT_FILE)
        os.remove(PCM_FILE)
    except:
        pass
    
if __name__ == '__main__':
    cleanFiles()
    generateLinkDef()

    cmd = "rootcling -f {} {} {}  > /dev/null 2>&1".format(DICT_FILE, HEADER_FILE, LINK_DEF_FILE)
    os.system(cmd)

    final_path = "{}".format(PCM_FILE)
    sys.stdout.write(final_path)
