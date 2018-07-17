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
import shutil
import sys

from print_command import Print
from new_command import InitializeNewGitRepo


DEMO_DIR = os.path.join(os.environ['BDM_INSTALL_DIR'], 'biodynamo', 'demo')
KNOWN_DEMOS = os.listdir(DEMO_DIR)


def DemoCommand(demo_name, destination=None):
    if not demo_name:
        print('Usage: biodynamo demo <demo name> [target directory]')
        print('Known demos:\n  {}'.format('\n  '.join(KNOWN_DEMOS)))
        return
    if demo_name not in KNOWN_DEMOS:
        Print.error('Demo name "{}" is not known.'.format(demo_name))
        print('Known demos:\n  {}'.format('\n  '.join(KNOWN_DEMOS)))
        sys.exit(1)
    if destination is None:
        destination = '.'
    if os.path.exists(destination):
        destination = os.path.join(destination, demo_name)
    if os.path.exists(destination):
        Print.error('Destination directory "{}" exists.'.format(destination))
        sys.exit(2)

    src_dir = os.path.join(DEMO_DIR, demo_name)
    print('Copying files from "{}" to "{}"...'.format(src_dir, destination))
    shutil.copytree(src_dir, destination)

    InitializeNewGitRepo(destination)

    Print.success('The demo "{}" has been created in "{}".'.format(
            demo_name, destination))
