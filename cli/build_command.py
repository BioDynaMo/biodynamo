import os, sys
import subprocess as sp
from print import Print


# The BioDynaMo CLI command to build a simulation binary.
def BuildCommand(clean=False, debug=False, build=True):
    build_dir = "build"
    debug_dir = "debug"


    if clean or debug:
        Print.new_step("Clean build directory")
        sp.check_output(["rm", "-rf", build_dir])
        sp.check_output(["mkdir", build_dir])
    else:
        if not os.path.exists(build_dir):
            sp.check_output(["mkdir", build_dir])

    if debug:
        if not os.path.exists(debug_dir):
            sp.check_output(["mkdir", debug_dir])

        with open(debug_dir + '/cmake_output.log', "w") as file:
            try:
                sp.check_call(
                    ["cmake", "-B./" + build_dir, "-H."],
                    stdout=file,
                    stderr=file)
            except sp.CalledProcessError as err:
                Print.error(
                "Failed to run CMake. Generating debug/cmake_output.log..."
                )
                return

        with open(debug_dir + '/make_output.log', "w") as file:
            try:
                sp.check_call(
                    ["make", "-C", build_dir],
                    stdout=file,
                    stderr=file)
            except sp.CalledProcessError as err:
                Print.error(
                "Compilation failed. Generating debug/make_output.log..."
                )
                return
                
    elif build:
        try:
            sp.check_output(["cmake", "-B./" + build_dir, "-H."])
        except sp.CalledProcessError as err:
            Print.error("Failed to run CMake. Check the debug output above.")
            sys.exit(1)

        try:
            sp.check_output(["make", "-j4", "-C", build_dir])
        except:
            Print.error("Compilation failed. Check the debug output above.")
            sys.exit(1)
