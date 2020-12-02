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

# USAGE: generate_version_files.py git_executable build_dir default_version git_dir
#   git_executable: e.g. git
#   build_dir: biodynamo cmake build directory
#   default_version: default version used if the git describe command fails
#   git_dir: the full path to the .git dir of the project

import os
import re
import subprocess
import sys

def VerifyCmdLineArguments():
    if not len(sys.argv) == 5:
        print("ERROR: generate_version_files.py expects four arguments")
        print("""
USAGE: generate_version_files.py git_executable build_dir default_version git_dir
  git_executable: e.g. git
  build_dir: biodynamo cmake build directory
  default_version: default version used if the git describe command fails
  git_dir: the full path to the .git dir of the project""")
        sys.exit(1)

def GetGitDescribeString(git_dir, default_version_code):
    try:
        version = subprocess.check_output('{0} --git-dir={1} describe --tags'.format(sys.argv[1], git_dir), stderr=subprocess.DEVNULL, shell=True).decode('utf-8')
        return version.strip()
    except subprocess.CalledProcessError as e:
        print("Call to git describe failed (Error code {})".format(e.returncode))
        version_string = "v{0}".format(default_version_code)
        print("Falling back to default version value ({})".format(version_string))
        return version_string

def GenerateFile(template_file, dest_file, version, major, minor, patch):
    if not os.path.exists(template_file):
        print("Warning: File {0} does not exist".format(template_file))
        return

    with open(template_file, 'r') as f:
        content = f.read()

    content = content.replace("@VERSION@", version)
    content = content.replace("@VERSION_MAJOR@", str(major))
    content = content.replace("@VERSION_MINOR@", str(minor))
    content = content.replace("@VERSION_PATCH@", str(patch))
    # for Doxyfile
    content = content.replace("PROJECT_NUMBER         = \"\"", "PROJECT_NUMBER         = \"" + version + "\"")

    with open(dest_file, 'w') as f:
        f.write(content)

def UpdateVersionInfo(version_file, version_string):
    """Returns true if the version_file does not exist (meaning that the
    script has not been executed yet), or if the last version does not match the
    current version."""

    if not os.path.exists(version_file):
        return True

    with open(version_file, 'r') as f:
        content = f.read()

    return version_string != content.strip()

if __name__ == '__main__':
    VerifyCmdLineArguments()

    version = GetGitDescribeString(sys.argv[4], sys.argv[3])

    # extract information
    search = re.search('v([0-9]+)\.([0-9]+)\-([0-9]+)\-(.*)', version)
    if search != None:
        major = search.group(1)
        minor = search.group(2)
        patch = search.group(3)
    else:
        search = re.search('v([0-9]+)\.([0-9]+)', version)
        major = search.group(1)
        minor = search.group(2)
        patch = 0

    # Update files
    scriptpath = os.path.dirname(__file__)
    destdir = sys.argv[2] + "/version"
    builddir = sys.argv[2]
    if not os.path.exists(destdir):
        os.makedirs(destdir)

    if UpdateVersionInfo(destdir+"/version", version):
        GenerateFile(scriptpath+'/version.h', destdir+'/version.h', version, major, minor, patch)
        GenerateFile(scriptpath+'/version.py', destdir+'/version.py', version, major, minor, patch)
        GenerateFile(builddir+'/Doxyfile', builddir+'/Doxyfile', version, major, minor, patch)

        # cache last version
        with open(destdir+"/version", 'w') as f:
            f.write(version)
        #   shortversion
        if patch == 0:
            shortversion = "{}.{}".format(major, minor)
        else:
            shortversion = "{}.{}.{}".format(major, minor, patch)
        with open(destdir+"/shortversion", 'w') as f:
            f.write(shortversion)
