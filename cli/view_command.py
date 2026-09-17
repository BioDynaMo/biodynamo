# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional new_steprmation
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

import os, sys, shutil
import subprocess as sp
from print_command import Print
from build_command import BuildCommand


## The BioDynaMo CLI command to execute the unit-tests created by default with
## the simulation template. Note that we ignore the configuration in bdm.json.
def ViewCommand(file_id=None):
    Print.success("<bdm view> Opening previous simulation results ...")
    # 1. Check if paraview is installed and if not, return
    bdmsys = os.environ.get("BDMSYS")
    if bdmsys is None:
        Print.error("<bdm view> BDMSYS is not set.")
        sys.exit(1)
    paraview_executable = os.path.join(
        bdmsys, "third_party/paraview/bin/paraview"
    )
    pv_found = sp.run("{} --version".format(paraview_executable), shell=True)
    if pv_found.returncode != 0:
        Print.error("<bdm view> ParaView not found.")
        sys.exit(1)

    # 2. attempt to change into project director if we are in build
    cwd = os.getcwd()
    if cwd.split("/")[-1] == "build":
        os.chdir("..")

    # 3. Find all pvsm files in output and build/output. Each entry records
    # whether it was found under build/output, since the pvsm stores its
    # data file references (e.g. "output/tumor_concept/MyCell-0.pvtu") as
    # paths relative to wherever `bdm run` was invoked from - so paraview
    # must be launched from that same directory to resolve them.
    pvsm_files = []
    if os.path.isdir("output"):
        for root, dirs, files in os.walk("output"):
            for file in files:
                if file.endswith(".pvsm"):
                    pvsm_files.append((os.path.join(root, file), False))
    if os.path.isdir("build/output"):
        for root, dirs, files in os.walk("build/output"):
            for file in files:
                if file.endswith(".pvsm"):
                    pvsm_files.append((os.path.join(root, file), True))

    # 5. Verify if there are any pvsm files; and that file_id is valid
    if len(pvsm_files) == 0:
        Print.error("<bdm view> No pvsm files found.")
        sys.exit(1)

    # 6. Convert all pvsm files to absolute paths
    pvsm_files = [(os.path.abspath(file), from_build) for file, from_build in pvsm_files]
    Print.new_step("<bdm view> Available PVSM files:")
    for i, (file, _) in enumerate(pvsm_files):
        print("<bdm view> File " + str(i + 1) + " : " + file)

    # 7. Choose the pvsm file
    Print.new_step("<bdm view> Selecting pvsm file.")
    if file_id is not None and file_id > len(pvsm_files):
        Print.error("<bdm view> Invalid file id ({}).".format(file_id))
        sys.exit(1)
    if file_id is None:
        print("<bdm view> By default, the latest pvsm file is opened.")
        print("<bdm view> Use 'bdm view <id>' to open another pvsm file.")
        pvsm_file, needs_build_cwd = max(pvsm_files, key=lambda f: os.path.getctime(f[0]))
    else:
        pvsm_file, needs_build_cwd = pvsm_files[file_id - 1]

    # 8. If this pvsm file was found under build/output, change to the
    #    build folder so its relative data file references resolve.
    #    NOTE: this used to check `if "build" in pvsm_file`, which misfires
    #    whenever any ancestor directory happens to be named "build" - most
    #    commonly BioDynaMo's own build/ directory, since that's where
    #    thisbdm.sh is sourced from. That sent paraview into the wrong
    #    directory (the simulation project's own CMake build folder) and
    #    its relative paths to the .pvtu/.pvti data then resolved to
    #    nonexistent locations, producing errors that a manual
    #    File > Load State (which lets you pick the right directory) does
    #    not hit.
    Print.new_step("<bdm view> Opening :" + pvsm_file)
    if needs_build_cwd:
        os.chdir("build")

    # 9. Open the latest pvsm file
    visualize = sp.run(
        "{} --state={}".format(paraview_executable, pvsm_file), shell=True
    )
    if visualize.returncode != 0:
        Print.warning(
            "<bdm view> Visualization terminated with return code {}.".format(
                visualize.returncode
            )
        )

    # 10. go back to initial folder.
    os.chdir(cwd)
    Print.success("<bdm view> Finished successfully.")
