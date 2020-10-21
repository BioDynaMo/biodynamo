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

import os, platform, time, sys
import subprocess as sp
from print_command import Print
from build_command import BuildCommand
from run_command import RunCommand
from util import GetBinaryName

def query_yes_no(question, default="yes"):
    """Ask a yes/no question via input() and return their answer.

    "question" is a string that is presented to the user.
    "default" is the presumed answer if the user just hits <Enter>.
    It must be "yes" (the default), "no" or None (meaning
    an answer is required of the user).

    The "answer" return value is True for "yes" or False for "no".
    """
    valid = {"yes": True, "y": True, "ye": True,
    "no": False, "n": False}
    if default is None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write(question + prompt)
        choice = input().lower()
        if default is not None and choice == '':
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' "
                             "(or 'y' or 'n').\n")

def UncommittedFiles():
    staged_files, unstaged_files = 0, 0
    untracked_files = ""
    Print.new_step("Check for uncommited files")

    try:
        sp.check_output(["git", "diff", "--cached", "--quiet"])
    except sp.CalledProcessError as err:
        staged_files = err.returncode

    try:
        sp.check_output(["git", "diff", "--quiet"])
    except sp.CalledProcessError as err:
        unstaged_files = err.returncode

    if not staged_files or not unstaged_files:
        try:
            untracked_files = sp.check_output(["git", "ls-files", "--exclude-standard", "--others"]).decode('ascii')
        except sp.CalledProcessError as err:
            Print.warning("Could not perform check on untracked files. Continuing anyway...")
            pass

    if staged_files or unstaged_files:
        if staged_files:
            print("You have staged files that need to be committed")
        if unstaged_files:
            print("You have unstaged files that need to be commited")


        print("")
        Print.warning("Rerun biodynamo assistance once you have resolved these issues")

        return 1

    if untracked_files != "":
        sp.call(["git", "status"])

        return not query_yes_no("You have untracked files. Are you sure they can be ignored?")

    return 0


def CopyRootDictionaries():
    sp.check_output(["cp", "build/*dict.log", "debug"])


def GenerateSystemInfoLog():
    Print.new_step("Generate system info log")

    try:
        sfile = open('debug/system_info.log', 'w+')
    except sp.CalledProcessError as err:
        print("Error: could not create system_info.log in directory 'debug'")
        sys.exit(1)

    my_os = platform.system()
    distro, version = "", ""

    architecture = platform.architecture()

    if my_os == "Linux":
        distro = platform.linux_distribution()[0]
        my_os = my_os + " " + distro + " " + platform.linux_distribution()[1]
    elif my_os == "Darwin":
        version = platform.mac_ver()[0]

    cmake_version = sp.check_output(["cmake", "--version"]).decode('ascii')
    cmake_version = cmake_version.split("\n")[0].split(" ")[-1].strip()
    # root_info = sp.check_output(
    # ["cat", "$ROOTSYS/include/RVersion.h", "|", "grep", "'ROOT_RELEASE '"])
    # root_version = root_info.split('"')[1::2][0]

    sfile.write("%-20s %10s\n" % ("Platform:", my_os))
    sfile.write("%-20s %10s\n" % ("Architecture:", architecture))

    sfile.write("")

    # sfile.write("%-20s %10s" % ("BioDynaMo version:", bdm_version))
    sfile.write("%-20s %10s\n" % ("CMake version:", cmake_version))
    # sfile.write("%-20s %10s" % ("ROOT version:", root_version))
    # sfile.write("%-20s %10s" % ("ParaView version:", paraview_version))

    sfile.close()


# Creates a folder "debug" with debug files
def GenerateDebugOutput(sim_name):
    Print.new_step("Generate debug output")
    # generates cmake_output.log and / or make_output.log (depends if cmake
    # ran sucessfully or not)
    if BuildCommand(debug=True):
        # generates runtime_output.log
        RunCommand(debug=True)
        # generates root dictionary log(s)
        CopyRootDictionaries()
    # generates system_info.log
    GenerateSystemInfoLog()


def CreateDebugBranch(sim_name):
    Print.new_step("Create debug branch")

    original_branch = sp.check_output(
        ["git", "rev-parse", "--abbrev-ref", "HEAD"]).decode('ascii').split("\n")[0]
    branch_name = "bdm-assist-" + time.strftime("%Y%m%d_%H%M")
    sp.check_output(["git", "checkout", "-b", branch_name])
    sp.check_output(["git", "add", "debug"])
    sp.check_output(["git", "commit", "-m", '"Add debug information"'])
    sp.check_output(["git", "push", "origin", branch_name])
    remote_url = sp.check_output(
        ["git", "remote", "get-url", "origin"]).decode('ascii').split("\n")[0]
    branch_url = remote_url[:-4] + "/tree/" + branch_name

    print("An assistance branch has been created, called '%s'" % (branch_name))
    print(
    "Send the following URL to the developers when requesting assistance:")
    print("")
    print(branch_url)

    print("Switching back to the previous branch...")
    sp.check_output(["git", "checkout", original_branch])


def AssistCommand():
    sim_name = GetBinaryName()
    if UncommittedFiles():
        return

    GenerateDebugOutput(sim_name)

    CreateDebugBranch(sim_name)
