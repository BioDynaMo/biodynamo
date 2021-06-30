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

import os
import subprocess as sp


## The BioDynaMo CLI command to execute the unit-tests created by default with
## the simulation template.
def TestCommand():
    cwd = os.getcwd()
    if cwd.split("/")[-1] == "build":
        sp.run("ctest", shell=True)
    else:
        os.chdir("build")
        sp.run("ctest", shell=True)
        os.chdir(cwd)
