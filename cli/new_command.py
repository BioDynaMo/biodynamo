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
import re

import subprocess as sp
import sys

from print_command import Print
from git_utils import *
from common import CopySupportFiles


def ValidateSimName(sim_name):
    pattern = re.compile("^[a-zA-Z]+[a-zA-Z0-9\-_]+$")
    if not pattern.match(sim_name):
        Print.error('Error: simulation name "{}" is not valid.'.format(sim_name))
        Print.error("       Allowed characters are a-z A-Z 0-9 - and _")
        Print.error("       Must start with a-z or A-Z")
        sys.exit(1)
    if sim_name.lower() == "test":
        Print.error('The directory name "test" is not allowed because it')
        Print.error("causes problems with the unit-test compilation.")
        Print.error("Please choose a different name.")
        sys.exit(1)
    if os.path.isdir(sim_name):
        Print.error('The directory "{}" already exists.'.format(sim_name))
        Print.error("Please remove it or choose a different name.")
        Print.error('Abort "biodynamo new {}".'.format(sim_name))
        sys.exit(1)


## Removes any created files during NewCommand and exits the program
def CleanupOnError(sim_name):
    try:
        sp.check_output(["rm", "-rf", sim_name])
    except:
        Print.error("Error: Failed to remove folder {0}".format(sim_name))
    sys.exit(1)


def CopyTemplate(sim_name):
    Print.new_step("Copy simulation template")
    try:
        src_path = "{0}/simulation-template".format(os.environ["BDMSYS"])
        sp.check_output(["cp", "-R", src_path, "./{0}".format(sim_name)])
    except sp.CalledProcessError as err:
        Print.error("Error while copying the template project.")
        # Do not use CleanupOnError here
        # One failure could be an already existing directory
        # we must not remove it
        sys.exit(1)


def ModifyFileContent(filename, fn):
    with open(filename) as f:
        content = f.read()

    content = fn(content)

    with open(filename, "w") as f:
        f.write(content)


def CustomizeFiles(sim_name):
    Print.new_step("Customize files")
    try:
        # include files
        include_guard = sim_name.upper().replace("-", "_") + "_H_"
        ModifyFileContent(
            sim_name + "/src/my-simulation.h",
            lambda c: c.replace("MY_SIMULATION_H_", include_guard),
        )

        # files whose content needs to be modified
        content_files = [
            "/README.md",
            "/CMakeLists.txt",
            "/src/my-simulation.cc",
            "/test/test-suite.cc",
        ]
        for file in content_files:
            ModifyFileContent(
                sim_name + file,
                lambda c: c.replace("my-simulation", sim_name),
            )

        #   rename
        os.rename(
            sim_name + "/src/my-simulation.h", sim_name + "/src/" + sim_name + ".h"
        )
        os.rename(
            sim_name + "/src/my-simulation.cc", sim_name + "/src/" + sim_name + ".cc"
        )
    except:
        Print.error("Error: File customizations failed")
        CleanupOnError(sim_name)

def CreateBuildDir(build_dir):
    sp.check_output(["mkdir", build_dir])

def NewCommand(sim_name, github):
    if github:
        print("Info: This command requires a Github.com account.")
        print("      Please have your account details ready, or ")
        print("      go over to https://github.com/join to sign up.")

    ValidateSimName(sim_name)
    CopyTemplate(sim_name)
    CopySupportFiles(sim_name)
    CustomizeFiles(sim_name)
    InitializeNewGitRepo(sim_name)
    CreateBuildDir("{}/build".format(sim_name))
    if github:
        CreateNewGithubRepository(sim_name)

    Print.success(sim_name + " has been created successfully!")
    print(
        "To compile and run this simulation, change the directory by calling "
        '"cd %s"' % (sim_name)
    )
