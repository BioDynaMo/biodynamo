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

import argparse
import os
import re
import subprocess as sp
import sys

import assist_command

def GetBinaryName():
    with open("CMakeLists.txt") as f:
        content = f.read()
        return re.search('project\((.*)\)', content).group(1)

# Retrieves the id of the current commit.
def GetCommitId():
  try:
    commit_id = sp.check_output(["git", "rev-parse", "HEAD"])
    return commit_id.decode('ascii')
  except sp.CalledProcessError as err:
    Print.error("Could not retrieve commit id")
    sys.exit(1)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='bdm-util',
        description='This is a BioDynaMo util script for use within the CLI or '
                    'usage in the benchmarking script',
        epilog='')

    spr = parser.add_subparsers(dest='cmd')
    commit_id_sp = spr.add_parser('commit-id', help='Get the commit ID of the current'
                            ' working directory.')

    args, unknown = parser.parse_known_args()

    if args.cmd == 'commit-id':
        if len(unknown) != 0:
            commit_id_sp.print_help()
            sys.exit(1)
        if (assist_command.UncommittedFiles() != 0):
          sys.exit(1)
        print(GetCommitId())
    else:
        parser.print_help()