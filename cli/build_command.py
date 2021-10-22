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

import os, sys
import subprocess as sp
from pathlib import Path
from print_command import Print
from util import RunProcessAndWriteToStdOut


# The BioDynaMo CLI command to build a simulation binary.
def BuildCommand(clean=False, build=True):
    build_dir = "build"

    Print.new_step("Build")

    if clean:
        Print.new_step("Clean build directory")
        sp.check_output(["rm", "-rf", build_dir])
        sp.check_output(["mkdir", build_dir])

    elif build:
        if not os.path.exists(build_dir):
            sp.check_output(["mkdir", build_dir])

        # if CMakeCache.txt does not exist, run cmake
        if not Path(build_dir + "/CMakeCache.txt").is_file():
            try:
                RunProcessAndWriteToStdOut(["cmake", "-B./" + build_dir, "-H."])
            except sp.CalledProcessError as err:
                Print.error(
                    "Failed to run CMake. Check the debug output above.")
                sys.exit(1)

        try:
            RunProcessAndWriteToStdOut(["make", "-j4", "-C", build_dir])
        except:
            Print.error("Compilation failed. Check the debug output above.")
            sys.exit(1)
