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

import os, platform
import subprocess as sp
from print_command import Print
from build_command import BuildCommand
from util import GetBinaryName

## The BioDynaMo CLI command to run a simulation
##
## @param      sim_name  The simulation name
##
def RunCommand(args):

    # 1. attempt to change into project director if we are in build
    cwd = os.getcwd()
    if cwd.split("/")[-1] == "build":
        os.chdir("..")
    sim_name = GetBinaryName()
    args_str = " ".join(args)
    cmd = "./build/" + sim_name

    BuildCommand()
    try:
        # 2. execute run command
        Print.new_step(
            "<bdm run> Running " + sim_name + " " + args_str + " ..."
        )
        if platform.system() == "Darwin":
            launcher = os.environ["BDMSYS"] + "/bin/launcher.sh"
            result = sp.run([launcher, cmd, args_str])
        else:
            result = sp.run([cmd, args_str])

        # 3. go back to initial folder.
        os.chdir(cwd)

        # 4. return the return code of the ctest command:
        if result.returncode != 0:
            Print.error(
                "<bdm run> Received return code {} from {}.".format(
                    result.returncode, cmd
                )
            )
            exit(result.returncode)
        Print.success("<bdm run> Finished successfully.")

    except sp.CalledProcessError as err:
        print(err.output.decode("utf-8"))
        Print.error("<bdm run> Error during execution of {0}".format(cmd))
        exit(1)
