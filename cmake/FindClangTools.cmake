#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Tries to find the clang-tidy and clang-format modules
#
# Usage of this module as follows:
#
#  find_package(ClangTools)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  ClangToolsBin_HOME -
#   When set, this path is inspected instead of standard library binary locations
#   to find clang-tidy and clang-format
#
# This module defines
#  CLANG_TIDY_BIN, The  path to the clang tidy binary
#  CLANG_TIDY_FOUND, Whether clang tidy was found
#  CLANG_FORMAT_BIN, The path to the clang format binary
#  CLANG_TIDY_FOUND, Whether clang format was found

find_program(CLANG_TIDY_BIN
  NAMES clang-tidy-4.0 clang-tidy-3.9 clang-tidy-3.8 clang-tidy
  PATHS ${ClangTools_PATH} $ENV{CLANG_TOOLS_PATH} /usr/local/bin /usr/bin
  NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
)

find_program(CLANG_FORMAT_BIN
  NAMES clang-format-4.0 clang-format-3.9 clang-format-3.8 clang-format
  PATHS ${ClangTools_PATH} $ENV{CLANG_TOOLS_PATH} /usr/local/bin /usr/bin
  NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
)

find_package_handle_standard_args(CLANG_TIDY DEFAULT_MSG
        CLANG_TIDY_BIN)
find_package_handle_standard_args(CLANG_FORMAT DEFAULT_MSG
        CLANG_FORMAT_BIN)
find_package_handle_standard_args(ClangTools DEFAULT_MSG
        CLANG_FORMAT_BIN CLANG_TIDY_BIN)


