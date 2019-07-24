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

from print_command import Print

def CopyStyleFiles(sim_name):
    STYLE_DIR = os.path.join(os.environ['BDM_INSTALL_DIR'], 'biodynamo', 'share', 'util', 'style_checks')
    Print.new_step("Copy clang-format code style files")
    for filename in os.listdir(STYLE_DIR):
        full_file_name = os.path.join(STYLE_DIR, filename)
        if os.path.isfile(full_file_name):
            shutil.copy(full_file_name, sim_name)
