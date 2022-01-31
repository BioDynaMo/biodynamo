# -----------------------------------------------------------------------------
#
# Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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
from print_command import Print
from pprint import pprint

# Converts a sting to a list by splitting it with the split_char.
def StringToList(input_string, split_char=" "):
    output_list = input_string.split(split_char)
    return output_list


# Converts a list of type ["key1=value1", ... ,"keyN=valueN"] to a dictionary of
# type {"key1":"value1", ... , "keyN" : "valueN"}.
def ListToDir(input_list):
    output_dir = {}
    for element in input_list:
        tmp_list = element.split("=")
        output_dir[tmp_list[0]] = tmp_list[1]
    return output_dir


## The BioDynaMo CLI command to print the configuration of biodynamo. Obtains
## information from <path_to_bdm>/build/bin/bdm-config
def ConfigCommand():
    Print.new_step("<bdm config> Loading BioDynaMo configuration ...")

    # 0. Define explanations
    explanation = {
        "arch": "The architecture (compiler/OS)",
        "platform": "The platform (OS)",
        "cxxflags": "Compiler flags and header path",
        "cxxincludes": "Only header paths (subset of cxxflags)",
        "ldflags": "Linker flags",
        "libs": "BioDynaMo libraries",
        "cmakedir": "BioDynaMo cmake directory",
        "bindir": "The executable directory",
        "libdir": "The library directory",
        "incdir": "The header directory",
        "config": "The cmake configuration options",
        "version": "The BioDynaMo version",
        "ncpu": "Number of available (hyperthreaded) cores",
        "cxx": "Alternative C++ compiler specified when BDM was built",
        "ld": "Alternative Linker specified when BDM was built",
        "cmake-invoke": "The BioDynaMo cmake invocation",
        "root-version": "The version of ROOT used to build BioDynaMo",
    }

    # 1. Get path to bdm-config
    bdm_config_command = os.path.join(os.environ["BDMSYS"], "bin", "bdm-config")

    # 2. Extract all information via bdm-config
    arguments = [
        "version",
        "root-version",
        "cxxflags",
        "cxxincludes",
        "ldflags",
        "libs",
        "bindir",
        "libdir",
        "incdir",
        "cxx",
        "ld",
        "cmakedir",
        "cmake-invoke",
        "config",
        "arch",
        "platform",
        "ncpu",
    ]
    bdm_config = {}
    for argument in arguments:
        result = sp.run(
            [bdm_config_command, "--{}".format(argument)], capture_output=True
        )
        bdm_config[argument] = result.stdout.decode("UTF-8").replace("\n", "")

    # 3. Format fields
    # 3.1 The following stings need to be converted to a list type.
    list_formats = [
        "cxxflags",
        "cxxincludes",
        "ldflags",
        "libs",
        "libdir",
        "incdir",
        "ld",
        "config",
    ]
    for lf in list_formats:
        bdm_config[lf] = StringToList(bdm_config[lf])
    # 3.2 The following lists need to be converted to a dictionary type.
    dict_formats = [
        "config",
    ]
    for df in dict_formats:
        bdm_config[df] = ListToDir(bdm_config[df])

    # 4. Print explanation and configuration
    for x in arguments:
        Print.new_step_in_config(x, explanation[x])
        pprint(bdm_config[x])

    # 5. Finalize
    Print.new_step("<bdm config> Finished successfully ...")
