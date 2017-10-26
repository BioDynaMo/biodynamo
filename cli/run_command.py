import os
import subprocess as sp
from print import Print

## The BioDynaMo CLI command to run a simulation
##
## @param      sim_name  The simulation name
##
def RunCommand():
    sim_name = os.getcwd().split("/")[-1]
    Print.new_step("Run " + sim_name)
    cmd = "./build/" + sim_name
    sp.check_output([cmd])


def RunWithDebugInfoCommand(sim_name):
    cmd = "./build/" + sim_name
    sp.check_output([cmd, "&>", "debug/runtime_output.log"])
