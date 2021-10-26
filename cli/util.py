# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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
import select


def GetBinaryName():
    with open("CMakeLists.txt") as f:
        content = f.read()
        return re.search("project\((.*)\)", content).group(1)

def RunProcessAndWriteToStdOut(cmd_args):
    p = sp.Popen(cmd_args, stdout=sp.PIPE, stderr=sp.PIPE)

    poll = select.poll()
    poll.register(p.stdout)
    poll.register(p.stderr)

    while p.poll() is None:
        rlist = poll.poll()
        for fd, event in rlist:
            sys.stdout.write(os.read(fd, 1024).decode('utf-8'))

