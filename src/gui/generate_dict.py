#!/usr/bin/env python3
#
# Author: Lukasz Stempniewicz 25/05/19
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
HEADER_FILE = '{}/view/model_creator.h'.format(DIR_PATH)
HEADER_FILE2 = '{}/controller/project_object.h'.format(DIR_PATH)
HEADER_FILE3 = '{}/model/model.h'.format(DIR_PATH)
INCLUDE_DIR = '{}/..'.format(DIR_PATH)

# Generated files
LINK_DEF_FILE = '{}/gui_LinkDef.h'.format(DIR_PATH)
DICT_FILE = '{}/gui_GEN_Dict.cc'.format(DIR_PATH)
PCM_FILE = '{}/gui_GEN_Dict_rdict.pcm'.format(DIR_PATH)
TMP_FILE = '{}/tmp.txt'.format(DIR_PATH)

# Not in use
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
        # os.remove(LINK_DEF_FILE)
        os.remove(DICT_FILE)
        os.remove(PCM_FILE)
    except:
        pass
    
if __name__ == '__main__':
    cleanFiles()
    # generateLinkDef()  Current not called

    cmd = "rootcling -f {} -I{} {} {} {} {}  > {} 2>&1".format(DICT_FILE, INCLUDE_DIR, HEADER_FILE, HEADER_FILE2, HEADER_FILE3, LINK_DEF_FILE, TMP_FILE)
    os.system(cmd)

    fp = open(TMP_FILE, "r")
    data = fp.read()
    fp.close()
    os.remove(TMP_FILE)

    if 'error' in data:
        errMsg = 'cmd {} \nErrored! Output: {}'.format(cmd, data)
        raise Exception(errMsg)

    final_path = "{}".format(PCM_FILE)
    sys.stdout.write(final_path)
