import argparse
from paraview.simple import *

parser = argparse.ArgumentParser(description='Command line arguments')
parser.add_argument('--raytracing', action='store_true', default=False)
parser.add_argument('--screenshots', action='store_true', default=False)
parser.add_argument('--animation', action='store_true', default=False)
params = parser.parse_args()

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# load state
LoadState('output/pyramidal_cell/pyramidal_cell.pvsm')

# find source
mySomas = FindSource('NeuronSomas')

# set active source
SetActiveSource(mySomas)

# Properties modified on mySomas.GlyphType
mySomas.GlyphType.ThetaResolution = 20
mySomas.GlyphType.PhiResolution = 20

# get active view
renderView1 = GetActiveViewOrCreate('RenderView')
# uncomment following to set a specific view size
# renderView1.ViewSize = [1452, 949]

# update the view to ensure updated data information
renderView1.Update()

# reset view to fit data
renderView1.ResetCamera()

# get animation scene
animationScene1 = GetAnimationScene()

animationScene1.GoToLast()

# reset view to fit data bounds
renderView1.ResetCamera(-4.98292255402, 4.98292255402, -4.98292255402, 4.98292255402, -5.0, 5.0)

# find source
myNeurites = FindSource('NeuriteElements')

# set active source
SetActiveSource(myNeurites)

# reset view to fit data bounds
renderView1.ResetCamera(-57.3483390808, 91.8613815308, -83.2577438354, 27.554019928, -125.748809814, 451.121826172)

# Hide orientation axes
renderView1.OrientationAxesVisibility = 0

# Properties modified on renderView1
renderView1.Background = [0.14901960784313725, 0.14901960784313725, 0.14901960784313725]

# Properties modified on myNeuritesDisplay
myNeuritesDisplay = GetDisplayProperties(myNeurites, view=renderView1)
myNeuritesDisplay.Luminosity = 10.0

# get the material library
materialLibrary1 = GetMaterialLibrary()

# current camera placement for renderView1
renderView1.CameraPosition = [-1024.8891945362295, 161.76612091064453, 154.97137451171875]
renderView1.CameraFocalPoint = [149.75026893615723, 161.76612091064453, 154.97137451171875]
renderView1.CameraViewUp = [0.0, 0.0, 1.0]
renderView1.CameraParallelScale = 304.0190642756604

# update the view to ensure updated data information
renderView1.Update()

if params.raytracing:
    pm = paraview.servermanager.vtkSMProxyManager
    if pm.GetVersionMajor() == 5 and pm.GetVersionMinor() < 7:
        renderView1.EnableOSPRay = 1
        renderView1.OSPRayRenderer = 'pathtracer'
    else:
        renderView1.EnableRayTracing = 1
        renderView1.BackEnd = 'OSPRay pathtracer'
        renderView1.Denoise = 1

    # Properties modified on renderView1
    renderView1.Shadows = 1

    # Properties modified on renderView1
    renderView1.SamplesPerPixel = 20

if params.screenshots:
    SaveScreenshot('output/pyramidal-cell-single.png', renderView1, ImageResolution=[1200, 4000], FontScaling='Scale fonts proportionally')
    print("Created image at: output/pyramidal-cell-single.png")

if params.animation:
    timesteps = GetAnimationScene().TimeKeeper.TimestepValues
    print(int(timesteps[-1]))

    SaveAnimation("output/animation.png", renderView1, ImageResolution=[1000, 2160],
        FontScaling='Scale fonts proportionally',
        OverrideColorPalette='',
        StereoMode='No change',
        TransparentBackground=0,
        FrameRate=1,
        FrameWindow=[0, int(timesteps[-1])],
        # PNG options
        CompressionLevel='5',
        SuffixFormat='.%04d')

