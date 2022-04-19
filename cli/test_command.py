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
## the simulation template. Note that we ignore the configuration in bdm.json.
def TestCommand():
    cwd = os.getcwd()
    # 1. attempt to change into project director if we are in build
    if cwd.split("/")[-1] == "build":
        os.chdir("..")
    # 2. Call build command to include latest changes of the repository
    BuildCommand()

    # 3. If present, rename bdm.json to bdm.json.bak (background: it is not
    #    helpful to run the tests with a modified bdm.json, outcome may change
    #    and test may fail)
    if os.path.isfile("bdm.json"):
        os.rename("bdm.json", "bdm.json.bak")

    # 4. Start testing
    Print.new_step("<bdm test> Running ctest ...")
    # 4.1 change into build directory
    try:
        os.chdir("build")
    except:
        Print.error("<bdm test> Failed to find build directory.")
        sys.exit(1)
    # 4.2 call "ctest"
    try:
        result = sp.run("ctest --output-on-failure", shell=True)
    except:
        Print.error("<bdm test> Calling the ctest command failed.")
        sys.exit(1)

    # 5. revert renaming of bdm.json
    os.chdir("..")
    if os.path.isfile("bdm.json.bak"):
        os.rename("bdm.json.bak", "bdm.json")

    # 6. go back to initial folder.
    os.chdir(cwd)

    # 7. return the return code of the ctest command:
    if result.returncode != 0:
        Print.error(
            "<bdm test> Received the ctest return code {}.".format(
                result.returncode
            )
        )
        exit(result.returncode)

    Print.success("<bdm test> Finished successfully.")
