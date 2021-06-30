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

import os, sys, shutil
import subprocess as sp
from print_command import Print
from build_command import BuildCommand


## The BioDynaMo CLI command to execute the unit-tests created by default with
## the simulation template.
def TestCommand():
    cwd = os.getcwd()
    if cwd.split("/")[-1] == "build":
        sp.run("ctest", shell=True)
    else:
        # If there is no build, we first build the simulation and tests
        if "build" not in os.listdir(cwd):
            try:
                BuildCommand()
                Print.new_step("Testing")
            except:
                Print.error("No build directory found and \"biodynamo build\"")
                Print.error("failed. Please build project first.")
                # Clean up if BuildCommand() fails.
                if "build" in os.listdir(cwd):
                    shutil.rmtree("build")
                sys.exit(1)
        # As soon as there is a build, we use it
        os.chdir("build")
        sp.run("ctest", shell=True)
        os.chdir(cwd)
