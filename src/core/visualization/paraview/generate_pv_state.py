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
from paraview.simple import *

def ExtractIterationFromFilename(x): return int(x.split('-')[-1].split('.')[0])

# ------------------------------------------------------------------------------
def ProcessSphere(so_info, so_data, render_view):
    # create a new 'Glyph'
    glyph_type = str(so_info['shape'])
    glyph1 = Glyph(Input=so_data, GlyphType=glyph_type)
    glyph1.GlyphTransform = 'Transform2'

    glyph1.GlyphType = glyph_type
    glyph1.ScaleFactor = 1.0
    glyph1.GlyphMode = 'All Points'
    #
    # # show data in view
    glyph1Display = Show(glyph1, render_view)
    # # trace defaults for the display properties.
    glyph1Display.Representation = 'Surface'
    glyph1Display.ColorArrayName = [None, '']
    glyph1Display.OSPRayScaleFunction = 'PiecewiseFunction'
    glyph1Display.SelectOrientationVectors = 'None'
    glyph1Display.ScaleFactor = 1.0
    glyph1Display.SelectScaleArray = 'None'
    glyph1Display.GlyphType = 'Sphere'
    glyph1Display.GlyphTableIndexArray = 'None'
    glyph1Display.DataAxesGrid = 'GridAxesRepresentation'
    glyph1Display.PolarAxes = 'PolarAxesRepresentation'
    glyph1Display.GaussianRadius = -1.0000000000000001e+298
    glyph1Display.SetScaleArray = [None, '']
    glyph1Display.ScaleTransferFunction = 'PiecewiseFunction'
    glyph1Display.OpacityArray = [None, '']
    glyph1Display.OpacityTransferFunction = 'PiecewiseFunction'

    # following statement causes:
    # Warning: In vtkSMPVRepresentationProxy.cxx, line 612
    # vtkSMPVRepresentationProxy (0x522db70): Failed to determine the
    # LookupTable being used.
    # glyph1Display.SetScalarBarVisibility(renderView1, True)

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on glyph1
    # ignored if set earlier
    if paraview.servermanager.vtkSMProxyManager.GetVersionMinor() == 5:
        glyph1.Scalars = ['POINTS', so_info['scaling_attribute']]
    else:
        glyph1.ScaleArray = ['POINTS', so_info['scaling_attribute']]
    RenameSource('{0}s'.format(so_info['name']), glyph1)

    # update the view to ensure updated data information
    render_view.Update()

# ------------------------------------------------------------------------------
def ProcessCylinder(so_info, so_data, render_view):
    glyph_type = str(so_info['shape'])
    bDMGlyph1 = BDMGlyph(Input=so_data, GlyphType=glyph_type)
    bDMGlyph1.Vectors = ['POINTS', 'None']
    bDMGlyph1.XScaling = ['POINTS', 'None']
    bDMGlyph1.YScaling = ['POINTS', 'None']
    bDMGlyph1.ZScaling = ['POINTS', 'None']
    bDMGlyph1.MassLocation = ['POINTS', 'None']
    bDMGlyph1.ScaleFactor = 0.1
    bDMGlyph1.GlyphTransform = 'Transform2'

    # show data in view
    bDMGlyph1Display = Show(bDMGlyph1, render_view)
    # trace defaults for the display properties.
    bDMGlyph1Display.Representation = 'Surface'
    bDMGlyph1Display.ColorArrayName = [None, '']
    bDMGlyph1Display.OSPRayScaleArray = 'actual_length_'
    bDMGlyph1Display.OSPRayScaleFunction = 'PiecewiseFunction'
    bDMGlyph1Display.SelectOrientationVectors = 'None'
    bDMGlyph1Display.ScaleFactor = 0.010000000149011612
    bDMGlyph1Display.SelectScaleArray = 'None'
    bDMGlyph1Display.GlyphType = glyph_type
    bDMGlyph1Display.GlyphTableIndexArray = 'None'
    bDMGlyph1Display.DataAxesGrid = 'GridAxesRepresentation'
    bDMGlyph1Display.PolarAxes = 'PolarAxesRepresentation'
    bDMGlyph1Display.GaussianRadius = 0.005000000074505806
    bDMGlyph1Display.SetScaleArray = ['POINTS', 'actual_length_']
    bDMGlyph1Display.ScaleTransferFunction = 'PiecewiseFunction'
    bDMGlyph1Display.OpacityArray = ['POINTS', 'actual_length_']
    bDMGlyph1Display.OpacityTransferFunction = 'PiecewiseFunction'

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.XScaling = ['POINTS', 'diameter_']

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.YScaling = ['POINTS', 'diameter_']

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.YScaling = ['POINTS', 'actual_length_']

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.ZScaling = ['POINTS', 'diameter_']

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.MassLocation = ['POINTS', 'mass_location_']

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.Vectors = ['POINTS', 'spring_axis_']

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.GlyphType = 'Cylinder'

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.ScaleMode = 'normal'

    # update the view to ensure updated data information
    render_view.Update()

    # Properties modified on bDMGlyph1
    bDMGlyph1.ScaleFactor = 1.0

    # update the view to ensure updated data information
    render_view.Update()

    bDMGlyph1.GlyphMode = 'All Points'
    RenameSource('{0}s'.format(so_info['name']), bDMGlyph1)

    render_view.Update()

# ------------------------------------------------------------------------------
def ProcessSimulationObject(result_dir, so_info):
    so_name = so_info['name']
    # determine pvtu files
    files = glob.glob('{0}/{1}-*.pvtu'.format(result_dir, so_name))
    if len(files) == 0:
        print('No data files found for simulation object {0}'.format(so_name))
        sys.exit(1)

    files.sort(cmp=lambda x, y: ExtractIterationFromFilename(x) - ExtractIterationFromFilename(y))


    # create a new 'XML Partitioned Unstructured Grid Reader'
    so_data = XMLPartitionedUnstructuredGridReader(FileName=files)
    # following line was in trace, but seems to be superfluous
    # so_data.PointArrayStatus = ['diameter_', 'volume_']

    # rename data source
    so_data_name = '{0}-data'.format(so_name)
    RenameSource(so_data_name, so_data)

    # get active view
    render_view = GetActiveViewOrCreate('RenderView')
    # get animation scene
    animation_scene = GetAnimationScene()
    # update animation scene based on data timesteps
    animation_scene.UpdateAnimationUsingDataTimeSteps()

    shape = str(so_info['shape'])
    if shape == "Sphere":
        ProcessSphere(so_info, so_data, render_view)
    elif shape == "Cylinder":
        ProcessCylinder(so_info, so_data, render_view)

    # reset view to fit data
    render_view.ResetCamera()

# ------------------------------------------------------------------------------
def AddDiffusionGradientGlyph(substance_name, substance_data, render_view):
    glyph1 = Glyph(Input=substance_data, GlyphType='Arrow')
    glyph1.ScaleFactor = 10
    glyph1.GlyphTransform = 'Transform2'

    # show data in view
    glyph1Display = Show(glyph1, render_view)
    # trace defaults for the display properties.
    glyph1Display.Representation = 'Surface'
    glyph1Display.ColorArrayName = [None, '']
    glyph1Display.OSPRayScaleArray = 'Diffusion Gradient'
    glyph1Display.OSPRayScaleFunction = 'PiecewiseFunction'
    glyph1Display.SelectOrientationVectors = 'GlyphVector'
    glyph1Display.ScaleFactor = 10
    glyph1Display.SelectScaleArray = 'Diffusion Gradient'
    glyph1Display.GlyphType = 'Arrow'
    glyph1Display.GlyphTableIndexArray = 'Diffusion Gradient'
    glyph1Display.DataAxesGrid = 'GridAxesRepresentation'
    glyph1Display.PolarAxes = 'PolarAxesRepresentation'
    glyph1Display.GaussianRadius = 9.73499984741211
    glyph1Display.SetScaleArray = ['POINTS', 'No scale array']
    glyph1Display.ScaleTransferFunction = 'PiecewiseFunction'
    glyph1Display.OpacityArray = ['POINTS', 'Substance Concentration']
    glyph1Display.OpacityTransferFunction = 'PiecewiseFunction'

    RenameSource('{0}-gradient'.format(substance_name), glyph1)

    # update the view to ensure updated data information
    render_view.Update()


# ------------------------------------------------------------------------------
def ProcessExtracellularSubstance(result_dir, substance_info):
    substance_name = substance_info['name']
    # determine pvti files
    files = glob.glob('{0}/{1}-*.pvti'.format(result_dir, substance_name))
    if len(files) == 0:
        print('No data files found for substance {0}'.format(substance_name))
        sys.exit(1)

    files.sort(cmp=lambda x, y: ExtractIterationFromFilename(x) - ExtractIterationFromFilename(y))

    substance_data = XMLPartitionedImageDataReader(FileName=files)
    substance_data.PointArrayStatus = ['Substance Concentration', 'Diffusion Gradient']

    # rename data source
    RenameSource('{0}-concentration'.format(substance_name), substance_data)

    # get active view
    renderView1 = GetActiveViewOrCreate('RenderView')
    # get animation scene
    animationScene1 = GetAnimationScene()
    # update animation scene based on data timesteps
    animationScene1.UpdateAnimationUsingDataTimeSteps()

    # get display properties
    substance_display = GetDisplayProperties(substance_data, view=renderView1)
    # set scalar coloring
    ColorBy(substance_display, ('POINTS', 'Substance Concentration', 'Magnitude'))
    # rescale color and/or opacity maps used to include current data range
    substance_display.RescaleTransferFunctionToDataRange(True, True)
    # change representation type
    substance_display.SetRepresentationType('Volume')
    # get color transfer function/color map for 'DiffusionGradient'
    diffusionGradientLUT = GetColorTransferFunction('DiffusionGradient')

    if substance_info['has_gradient'] == "true":
        AddDiffusionGradientGlyph(substance_name, substance_data, renderView1)

    # reset view to fit data
    renderView1.ResetCamera()


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

    # simulation objects
    for so_info in build_info['sim_objects']:
        ProcessSimulationObject(result_dir, so_info)
    # extracellular substances
    for substance_info in build_info['extracellular_substances']:
        ProcessExtracellularSubstance(result_dir, substance_info)

    os.chdir(result_dir)
    SaveState('{0}.pvsm'.format(sim_info['name']))


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
