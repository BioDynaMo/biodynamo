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

def ExtractIterationFromFilename(x): return int(x.split('_')[-1].split('.')[0])

# ------------------------------------------------------------------------------
def ProcessSimulationObject(so_info):
    so_name = so_info['name']
    # determine pvtu files
    files = glob.glob('./{0}s_data*.pvtu'.format(so_name))
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
    renderView1 = GetActiveViewOrCreate('RenderView')
    # get animation scene
    animationScene1 = GetAnimationScene()
    # update animation scene based on data timesteps
    animationScene1.UpdateAnimationUsingDataTimeSteps()

    # create a new 'Glyph'
    glyph_type = str(so_info['shape'])
    glyph1 = Glyph(Input=so_data, GlyphType=glyph_type)
    glyph1.Scalars = ['POINTS', 'None']
    glyph1.Vectors = ['POINTS', 'None']
    glyph1.GlyphTransform = 'Transform2'

    glyph1.GlyphType = glyph_type
    glyph1.ScaleMode = 'scalar'
    glyph1.ScaleFactor = 1.0
    glyph1.GlyphMode = 'All Points'
    #
    # # show data in view
    glyph1Display = Show(glyph1, renderView1)
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
    renderView1.Update()

    # Properties modified on glyph1
    # ignored if set earlier
    glyph1.Scalars = ['POINTS', so_info['scaling_attribute']]
    RenameSource('{0}s'.format(so_info['name']), glyph1)

    # update the view to ensure updated data information
    renderView1.Update()

    # reset view to fit data
    renderView1.ResetCamera()

# ------------------------------------------------------------------------------
def AddDiffusionGradientGlyph(substance_name, substance_data, render_view):
    glyph1 = Glyph(Input=substance_data, GlyphType='Arrow')
    glyph1.Scalars = ['POINTS', 'None']
    glyph1.Vectors = ['POINTS', 'Diffusion Gradient']
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
    glyph1Display.SetScaleArray = ['POINTS', 'Substance Concentration']
    glyph1Display.ScaleTransferFunction = 'PiecewiseFunction'
    glyph1Display.OpacityArray = ['POINTS', 'Substance Concentration']
    glyph1Display.OpacityTransferFunction = 'PiecewiseFunction'

    RenameSource('{0}-gradient'.format(substance_name), glyph1)

    # update the view to ensure updated data information
    render_view.Update()


# ------------------------------------------------------------------------------
def ProcessExtracellularSubstance(substance_info):
    substance_name = substance_info['name']
    # determine pvti files
    files = glob.glob('./{0}*.pvti'.format(substance_name))
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
    ColorBy(substance_display, ('POINTS', 'Diffusion Gradient', 'Magnitude'))
    # rescale color and/or opacity maps used to include current data range
    substance_display.RescaleTransferFunctionToDataRange(True, True)
    # change representation type
    substance_display.SetRepresentationType('Volume')
    # get color transfer function/color map for 'DiffusionGradient'
    diffusionGradientLUT = GetColorTransferFunction('DiffusionGradient')

    if substance_info['has_gradient']:
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
    os.chdir(result_dir)

    # simulation objects
    for so_info in build_info['sim_objects']:
        ProcessSimulationObject(so_info)
    # extracellular substances
    for substance_info in build_info['extracellular_substances']:
        ProcessExtracellularSubstance(substance_info)

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
