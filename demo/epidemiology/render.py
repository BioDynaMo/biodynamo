import argparse
import os
import sys
from paraview.simple import *
from paraview import coprocessing

def ExtendDefaultPipeline(renderview, coprocessor, datadescription, script_args):
    parser = argparse.ArgumentParser(description='BDM-PV-Params')
    parser.add_argument('--raytracing', action='store_true')
    parser.add_argument('--screenshots', action='store_true')
    parser.add_argument('--name', action='store', dest="name")
    params = parser.parse_args(script_args)
    print(script_args)
    print(params)
    print(params.raytracing)
    # find source
    persons = FindSource('Persons')

    # set active source
    SetActiveSource(persons)

    # get active view
    renderView1 = renderview 
    # uncomment following to set a specific view size
    # renderView1.ViewSize = [3150, 1350]

    # create a new 'Threshold' to filter out agent that was created as a UniformGridEnvironement bug workaround
    threshold1 = Threshold(Input=persons)
    threshold1.Scalars = ['POINTS', 'diameter_']
    threshold1.ThresholdRange = [2.0, 2.0]
    SetActiveSource(threshold1)
    Hide(persons)

    # get layout
    layout1 = GetLayout()

    # get display properties
    agentsDisplay = GetDisplayProperties(threshold1, view=renderView1)

    # set scalar coloring
    ColorBy(agentsDisplay, ('POINTS', 'state_'))

    # rescale color and/or opacity maps used to include current data range
    agentsDisplay.RescaleTransferFunctionToDataRange(True, False)

    # show color bar/color legend
    agentsDisplay.SetScalarBarVisibility(renderView1, True)

    # get color transfer function/color map for 'state_'
    state_LUT = GetColorTransferFunction('state_')
    state_LUT.ScalarRangeInitialized = 1.0

    # get opacity transfer function/opacity map for 'state_'
    state_PWF = GetOpacityTransferFunction('state_')
    state_PWF.ScalarRangeInitialized = 1

    # rescale color and/or opacity maps used to exactly fit the current data range
    agentsDisplay.RescaleTransferFunctionToDataRange(False, True)

    # Rescale transfer function
    state_LUT.RescaleTransferFunction(0.0, 2.0)

    # Rescale transfer function
    state_PWF.RescaleTransferFunction(0.0, 2.0)

    # Properties modified on state_LUT
    state_LUT.RGBPoints = [0.0, 0.6470588235294118, 0.37254901960784315, 0.09019607843137255, 1.0, 0.865003, 0.865003, 0.865003, 2.0, 0.705882, 0.0156863, 0.14902]

    # Properties modified on state_LUT
    state_LUT.RGBPoints = [0.0, 0.6470588235294118, 0.37254901960784315, 0.09019607843137255, 1.0, 0.7568627450980392, 0.18823529411764706, 0.1803921568627451, 2.0, 0.705882, 0.0156863, 0.14902]

    # Properties modified on state_LUT
    state_LUT.RGBPoints = [0.0, 0.6470588235294118, 0.37254901960784315, 0.09019607843137255, 1.0, 0.7568627450980392, 0.18823529411764706, 0.1803921568627451, 2.0, 0.0392156862745098, 0.48627450980392156, 0.5882352941176471]

    # Properties modified on state_LUT
    state_LUT.RGBPoints = [0.0, 0.4392156862745098, 0.43137254901960786, 0.43529411764705883, 1.0, 0.7568627450980392, 0.18823529411764706, 0.1803921568627451, 2.0, 0.0392156862745098, 0.48627450980392156, 0.5882352941176471]

    # Properties modified on state_LUT
    state_LUT.RGBPoints = [0.0, 1.0, 1.0, 1.0, 1.0, 0.7568627450980392, 0.18823529411764706, 0.1803921568627451, 2.0, 0.0392156862745098, 0.48627450980392156, 0.5882352941176471]

    if params.raytracing:
        renderView1.EnableRayTracing = 1
        renderView1.BackEnd = 'OSPRay pathtracer'
        renderView1.Denoise = 1
        renderView1.SamplesPerPixel = 10

    renderView1.Background = [0.14901960784313725, 0.14901960784313725, 0.14901960784313725]

    # Hide orientation axes
    renderView1.OrientationAxesVisibility = 0

    # hide color bar/color legend
    agentsDisplay.SetScalarBarVisibility(renderView1, False)

    renderView1.CameraPosition = [145.5451553636775, 185.21878354571766, 348.0277030120568]
    renderView1.CameraFocalPoint = [50.21011908104193, 43.8733056934362, 52.84622954370056] 
    renderView1.CameraViewUp = [-0.14570586437890107, 0.9098135966908945, -0.38859879099397066] 
    renderView1.CameraParallelScale = 88.22632763603929 
    #  
    #  renderView1.ResetCamera()
    timesteps = GetAnimationScene().TimeKeeper.TimestepValues

    #  SaveScreenshot('epidemiology.png', renderView1, ImageResolution=[2000, 2000],
    #      FontScaling='Scale fonts proportionally',
    #      OverrideColorPalette='',
    #      StereoMode='No change',
    #      # PNG options
    #      CompressionLevel='5')
    SaveAnimation("{}.png".format(params.name), renderView1, ImageResolution=[1024, 1024],
        FontScaling='Scale fonts proportionally',
        OverrideColorPalette='',
        StereoMode='No change',
        TransparentBackground=0,
        FrameRate=1,
        FrameWindow=[0, int(timesteps[-1])],
        # PNG options
        CompressionLevel='5',
        SuffixFormat='.%04d')
    print("Images ({}.*.png) created in directory: {}".format(params.name, os.getcwd()))


# Entry point for export visualization
if __name__ == "__main__":
    #### disable automatic camera reset on 'Show'
    paraview.simple._DisableFirstRenderCameraReset()
    sys.path.insert(0, "{0}/include/core/visualization/paraview".format(os.environ['BDMSYS']))
    from generate_pv_state import BuildDefaultPipeline
    BuildDefaultPipeline('output/epidemiology1/simulation_info.json')
    ExtendDefaultPipeline(GetActiveViewOrCreate('RenderView'), None, None, sys.argv[2:]) 

