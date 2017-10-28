import os
import subprocess as sp
from print_command import Print
from build_command import BuildCommand


## The BioDynaMo CLI command to run a simulation
##
## @param      sim_name  The simulation name
##
def RunCommand(debug=False):
    sim_name = os.getcwd().split("/")[-1]
    cmd = "./build/" + sim_name

    try:
        BuildCommand()
        Print.new_step("Run " + sim_name)
        if debug:
            sp.check_output([cmd, "&>", "debug/runtime_output.log"])
        else:
            print(sp.check_output([cmd], stderr=sp.STDOUT).decode('utf-8'))
            Print.success("Finished successfully")
    except sp.CalledProcessError as err:
        print(err.output.decode("utf-8"))
        Print.error("Error during execution of {0}".format(cmd))
