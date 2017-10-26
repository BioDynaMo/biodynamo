import os
import subprocess as sp
from print import Print
from build_command import BuildCommand
from pathlib import Path


## The BioDynaMo CLI command to run a simulation
##
## @param      sim_name  The simulation name
##
def RunCommand(debug=False):
    sim_name = os.getcwd().split("/")[-1]
    Print.new_step("Run " + sim_name)
    cmd = "./build/" + sim_name
    if(Path(cmd).is_file()):
        try:
            if debug:
                sp.check_output([cmd, "&>", "debug/runtime_output.log"])
            else:
                sp.check_output([cmd])
                Print.success("Finished successfully")
        except sp.CalledProcessError as err:
            Print.error("Error during execution of {0}".format(cmd))
    else:
        Print.warning("Could not find executable. Will start to build project.")
        BuildCommand()
