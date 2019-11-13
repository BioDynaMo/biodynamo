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

# add a target to generate BioDynaMo website using Gatsby static generator


set(WEB_DIR ${CMAKE_CURRENT_BINARY_DIR}/website)
add_custom_target(website
    COMMAND rm -rf ${WEB_DIR}
    COMMAND git clone https://github.com/BioDynaMo/website.git ${WEB_DIR}
    WORKING_DIRECTORY "${WEB_DIR}"
    COMMENT "Generate website"
    COMMAND ./build_website --api
    VERBATIM)

# ------------------------------------------------------------------------------
