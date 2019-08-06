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
import traceback
import subprocess as sp
from print_command import Print

## The BioDynaMo CLI command to start a gui
##
def GuiCommand(args):
    args_str = ' '.join(args)
    try:
        Print.new_step("Starting GUI with args: " + args_str)
        gui_bin = os.path.join(os.environ['BDM_INSTALL_DIR'], 'bin', 'gui')
        gui_args = os.path.join(os.environ['BDM_INSTALL_DIR'], 'log')
        if not os.path.exists(gui_args):
            os.makedirs(gui_args)
        gui_args = os.path.join(gui_args, "gui.log") 
        sp.run([gui_bin, gui_args])
        Print.success("Exited GUI successfully")
    except Exception as e:
        Print.error(traceback.format_exc())
