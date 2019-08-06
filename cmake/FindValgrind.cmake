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
# - VALGRIND_FOUND: if we were able to find valgrind's executable;
# - VALGRIND_PROGRAM: it will keep the location of valgrind's executable.

find_program(VALGRIND_PROGRAM NAMES valgrind PATH
    /usr/bin /usr/local/bin)
find_package_handle_standard_args(VALGRIND DEFAULT_MSG
    VALGRIND_PROGRAM)