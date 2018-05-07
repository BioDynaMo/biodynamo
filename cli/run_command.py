import os
import subprocess as sp
from print_command import Print
from build_command import BuildCommand
from util import GetBinaryName

## The BioDynaMo CLI command to run a simulation
##
## @param      sim_name  The simulation name
##
def RunCommand(args, debug=False):
    sim_name = GetBinaryName()
    args_str = ' '.join(args)
    cmd = "./build/" + sim_name

    try:
        BuildCommand()
        Print.new_step("Run " + sim_name + ' ' + args_str)
        if debug:
            sp.check_output([cmd, "&>", "debug/runtime_output.log"])
        else:
            print(sp.check_output([cmd, args_str], stderr=sp.STDOUT).decode('utf-8'))
            Print.success("Finished successfully")
    except sp.CalledProcessError as err:
        print(err.output.decode("utf-8"))
        Print.error("Error during execution of {0}".format(cmd))
