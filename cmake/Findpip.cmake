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
#
# This CMake script will define these two variables:
# - pip_FOUND: if we were able to find pip's executable;
# - pip_PROGRAM: it will keep the location of pip's executable.

find_program(pip_PROGRAM NAMES pip pip3 PATH
        /usr/bin /usr/local/bin)
find_package_handle_standard_args(pip DEFAULT_MSG
        pip_PROGRAM)