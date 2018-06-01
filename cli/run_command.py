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
import subprocess as sp
from print_command import Print
from build_command import BuildCommand
from util import GetBinaryName

## The BioDynaMo CLI command to run a simulation
##
## @param      sim_name  The simulation name
##
def RunCommand(args, debug=False):
    sim_name = GetBinaryName()
    args_str = ' '.join(args)
    cmd = "./build/" + sim_name

    try:
        BuildCommand()
        Print.new_step("Run " + sim_name + ' ' + args_str)
        if debug:
            sp.check_output([cmd, "&>", "debug/runtime_output.log"])
        else:
            print(sp.check_output([cmd, args_str], stderr=sp.STDOUT).decode('utf-8'))
            Print.success("Finished successfully")
    except sp.CalledProcessError as err:
        print(err.output.decode("utf-8"))
        Print.error("Error during execution of {0}".format(cmd))
