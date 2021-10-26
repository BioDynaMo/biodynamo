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

import os, sys, shutil
import subprocess as sp
from print_command import Print
from build_command import BuildCommand


## The BioDynaMo CLI command to execute the unit-tests created by default with
## the simulation template.
def TestCommand():
    cwd = os.getcwd()
    # 1. attempt to change into project director if we are in build
    if cwd.split("/")[-1] == "build":
        os.chdir("..")
    # 2. Call build command to include latest changes of the repository
    try:
        BuildCommand()
    except:
        Print.error(
            "The build command failed. Make sure you're in the project folder"
        )
        sys.exit(1)

    # 3. Start testing
    Print.new_step("Testing")
    # 3.1 change into build directory
    try:
        os.chdir("build")
    except:
        Print.error("Failed to find build directory.")
        sys.exit(1)
    # 3.2 call "ctest"
    try:
        sp.run("ctest", shell=True)
    except:
        Print.error("Calling the ctest test command failed.")
        sys.exit(1)

    # 4. go back to initial folder.
    os.chdir(cwd)
