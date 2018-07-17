#!/usr/bin/env python3
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

# USAGE: generate_version_files.py git_executable build_dir
#   git_executable e.g. git
#   build_dir biodynamo cmake build directory

import os
import re
import subprocess
import sys

def VerifyCmdLineArguments():
    if not len(sys.argv) == 3:
        print("ERROR: generate_version_files.py expects two arguments")
        print("""
USAGE: generate_version_files.py git_executable build_dir
  git_executable e.g. git
  build_dir biodynamo cmake build directory""")
        sys.exit(1)

def GetGitDescribeString():
    try:
        version = subprocess.check_output('{0} describe --tags'.format(sys.argv[1]), stderr=subprocess.DEVNULL, shell=True).decode('utf-8')
        return version.strip()
    except subprocess.CalledProcessError as e:
        print("Call to git describe failed")
        print("Error code", e.returncode, e.output)
        sys.exit(1)

def GenerateFile(template_file, dest_file, version, major, minor, patch, additional_commits):
    with open(template_file, 'r') as f:
        content = f.read()

    content = content.replace("@VERSION@", version)
    content = content.replace("@VERSION_MAJOR@", major)
    content = content.replace("@VERSION_MINOR@", minor)
    content = content.replace("@VERSION_PATCH@", patch)
    content = content.replace("@VERSION_ADDITIONAL_COMMITS@", additional_commits)
    content = content.replace("PROJECT_NUMBER         = \"\"", "PROJECT_NUMBER         = \"" + version + "\"")

    with open(dest_file, 'w') as f:
        f.write(content)

def UpdateVersionInfo(last_version_file, version_string):
    """Returns true if the last_version_file does not exist (meaning that the
    script has not been executed yet), or if the last version does not match the
    current version."""

    if not os.path.exists(last_version_file):
        return True

    with open(last_version_file, 'r') as f:
        content = f.read()

    return version_string != content.strip()

if __name__ == '__main__':
    VerifyCmdLineArguments()

    version = GetGitDescribeString()

    # extract information
    search = re.search('v([0-9]+)\.([0-9]+)\.([0-9]+)\-([0-9]+)\-(.*)', version)
    major = search.group(1)
    minor = search.group(2)
    patch = search.group(3)
    additional_commits = search.group(4)

    # Update files
    scriptpath = os.path.dirname(__file__)
    destdir = sys.argv[2] + "/version"
    builddir = sys.argv[2]
    if not os.path.exists(destdir):
        os.makedirs(destdir)

    if UpdateVersionInfo(destdir+"/last_version", version):
        GenerateFile(scriptpath+'/version.h', destdir+'/version.h', version, major, minor, patch, additional_commits)
        GenerateFile(scriptpath+'/version.py', destdir+'/version.py', version, major, minor, patch, additional_commits)
        GenerateFile(builddir+'/Doxyfile', builddir+'/Doxyfile', version, major, minor, patch, additional_commits)

        # cache last version
        with open(destdir+"/last_version", 'w') as f:
            f.write(version)
