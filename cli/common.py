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
import shutil

from print_command import Print


def CopySupportFiles(sim_name):
    SUPPORT_DIR = os.path.join(os.environ["BDMSYS"], "share", "util",
                               "support_files")
    Print.new_step("Copy additional support files")
    for filename in os.listdir(SUPPORT_DIR):
        full_file_name = os.path.join(SUPPORT_DIR, filename)
        if os.path.isfile(full_file_name):
            shutil.copy(full_file_name, sim_name)
