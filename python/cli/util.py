# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

import argparse
import os
import re
import subprocess as sp
import sys

def GetBinaryName():
    with open("CMakeLists.txt") as f:
        content = f.read()
        return re.search('project\((.*)\)', content).group(1)
