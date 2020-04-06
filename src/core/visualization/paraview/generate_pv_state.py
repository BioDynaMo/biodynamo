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

# This python script generates the python state from the exported files
# Therefore, the user can load the visualization simply by opening the pvsm file
# ARGUMENT: json_filename
#   json_filename: simulation information required to generate the state
#     sample files
#        {
#          "simulation": {
#              "name":"cancergrowth",
#              "result_dir":"/tmp/simulation-templates/diffusion"
#          },
#          "sim_objects": [
#            { "name":"cell", "glyph":"Glyph", "shape":"Sphere", "scaling_attribute":"diameter_" }
#          ],
#          "extracellular_substances": [
#            { "name":"Kalium", "has_gradient":"true" }
#          ]
#        }

import glob
import json
import os.path
import sys
import functools
from paraview.simple import *
from default_insitu_pipeline import *

def ExtractIterationFromFilename(x): return int(x.split('-')[-1].split('.')[0])

# ------------------------------------------------------------------------------
def LoadSimulationObjectData(result_dir, so_info):
    so_name = so_info['name']
    # determine pvtu files
    files = glob.glob('{0}/{1}-*.pvtu'.format(result_dir, so_name))
    if len(files) == 0:
        print('No data files found for simulation object {0}'.format(so_name))
        sys.exit(1)

    files = sorted(files, key=functools.cmp_to_key(lambda x, y: ExtractIterationFromFilename(x) - ExtractIterationFromFilename(y)))

    # create a new 'XML Partitioned Unstructured Grid Reader'
    return XMLPartitionedUnstructuredGridReader(FileName=files)

# ------------------------------------------------------------------------------
def LoadExtracellularSubstanceData(result_dir, substance_info):
    substance_name = substance_info['name']
    # determine pvti files
    files = glob.glob('{0}/{1}-*.pvti'.format(result_dir, substance_name))
    if len(files) == 0:
        print('No data files found for substance {0}'.format(substance_name))
        sys.exit(1)

    files = sorted(files, key=functools.cmp_to_key(lambda x, y: ExtractIterationFromFilename(x) - ExtractIterationFromFilename(y)))
    return XMLPartitionedImageDataReader(FileName=files)

# ------------------------------------------------------------------------------
def BuildParaviewState(build_info):
    #### disable automatic camera reset on 'Show'
    paraview.simple._DisableFirstRenderCameraReset()

    sim_info = build_info['simulation']

    # change directory
    result_dir = sim_info['result_dir']
    if result_dir != "" and not os.path.exists(result_dir):
        print('Simulation result directory "{0}" does not exist'.format(result_dir))
        sys.exit(1)

    # get active view
    render_view = GetActiveViewOrCreate('RenderView')
    render_view.InteractionMode = '3D'

    # simulation objects
    for so_info in build_info['sim_objects']:
        data = LoadSimulationObjectData(result_dir, so_info)
        ProcessSimulationObject(so_info, data, render_view)
    # extracellular substances
    for substance_info in build_info['extracellular_substances']:
        data = LoadExtracellularSubstanceData(result_dir, substance_info)
        ProcessExtracellularSubstance(substance_info, data, render_view)

    # get animation scene
    animation_scene = GetAnimationScene()
    # update animation scene based on data timesteps
    animation_scene.UpdateAnimationUsingDataTimeSteps()

    os.chdir(result_dir)
    SaveState('{0}.pvsm'.format(sim_info['name']))

    # This avoid the error: Inconsistency detected by ld.so
    # See: https://discourse.paraview.org/t/inconsistency-detected-by-ld-so/3778
    Show(Cone())


# ------------------------------------------------------------------------------
if __name__ == '__main__':
    arguments = sys.argv[1:]
    if len(arguments) != 1:
        print("This script expects the json filename as argument.")
        sys.exit(1)
    json_filename = arguments[0]

    # load json file containing the information to generate the state
    if os.path.exists(json_filename):
        with open(json_filename, 'r') as json_file:
            build_info = json.load(json_file)
    else:
        print('Json file {0} does not exist'.format(json_filename))
        sys.exit(1)

    BuildParaviewState(build_info)
