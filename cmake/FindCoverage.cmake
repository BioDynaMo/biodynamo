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
include(FindPackageHandleStandardArgs)

find_program(GCOV_PATH gcov)
find_program(LCOV_PATH lcov)
find_program(GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/tests)

find_program(GENINFO_PATH geninfo)
find_program(GENHTML_PATH genhtml)
find_package_handle_standard_args(Coverage DEFAULT_MSG
        GCOV_PATH LCOV_PATH GENINFO_PATH GENHTML_PATH
)