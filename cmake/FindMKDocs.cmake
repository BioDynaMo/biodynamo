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
# - MKDocs_FOUND: if we were able to find mkdocs's executable;
# - MKDOCS_PROGRAM: it will keep the location of mkdocs's executable.

find_program(MKDOCS_PROGRAM NAMES mkdocs
  PATHS /usr/local/bin ~/.local/bin ~/Library/Python/2.7/bin
)
find_package_handle_standard_args(MKDocs DEFAULT_MSG MKDOCS_PROGRAM)
