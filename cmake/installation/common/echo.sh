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

# codes taken from print_command.py
BDM_ECHO_PURPLE='\033[95m'
BDM_ECHO_CYAN='\033[96m'
BDM_ECHO_DARKCYAN='\033[36m'
BDM_ECHO_BLUE='\033[94m'
BDM_ECHO_GREEN='\033[92m'
BDM_ECHO_YELLOW='\033[93m'
BDM_ECHO_RED='\033[91m'
BDM_ECHO_BOLD='\033[1m'
BDM_ECHO_UNDERLINE='\033[4m'
BDM_ECHO_RESET='\033[0m'

function EchoSuccess {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_GREEN}$@${BDM_ECHO_RESET}"
}

function EchoError {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_RED}$@${BDM_ECHO_RESET}"
}

function EchoWarning {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_YELLOW}$@${BDM_ECHO_RESET}"
}

function EchoNewStep {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_BLUE}$@${BDM_ECHO_RESET}"
}
