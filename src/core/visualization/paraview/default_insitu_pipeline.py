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

from paraview.simple import *
from paraview import vtk
from paraview import coprocessing
import json

# ------------------------------------------------------------------------------
# Helper functions -------------------------------------------------------------
# ------------------------------------------------------------------------------
is_insitu_pipeline = False

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
def ProcessSimulationObject(so_info, so_data, render_view):
    # following line was in trace, but seems to be superfluous
    # so_data.PointArrayStatus = ['diameter_', 'volume_']

    # rename data source
    so_name = so_info['name']
    so_data_name = '{0}-data'.format(so_name)
    RenameSource(so_data_name, so_data)

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
def ProcessExtracellularSubstance(substance_info, substance_data, render_view):
    # get display properties
    substance_display = Show(substance_data, render_view)
    # set scalar coloring
    ColorBy(substance_display, ('POINTS', 'Substance Concentration', 'Magnitude'))
    # rescale color and/or opacity maps used to include current data range
    substance_display.RescaleTransferFunctionToDataRange(True, True)
    # change representation type
    # NB: Paraview v5.6.0 screenshots from within catalyst don't render volume
    #     rendering. -> Change default to Wireframe
    if is_insitu_pipeline:
        substance_display.SetRepresentationType('Wireframe')
    else:
        substance_display.SetRepresentationType('Volume')
    # get color transfer function/color map for 'DiffusionGradient'
    diffusionGradientLUT = GetColorTransferFunction('DiffusionGradient')

    # rename data source
    substance_name = substance_info['name']
    RenameSource('{0}-concentration'.format(substance_name), substance_data)

    if substance_info['has_gradient'] == "true":
        AddDiffusionGradientGlyph(substance_name, substance_data, render_view)

# ------------------------------------------------------------------------------
# CoProcessor definition -------------------------------------------------------
# ------------------------------------------------------------------------------
def CreateCoProcessor():
    def _CreatePipeline(coprocessor, datadescription):
        class Pipeline :
            #disable automatic camera reset on 'Show'
            paraview.simple._DisableFirstRenderCameraReset()

            renderview = CreateView('RenderView')
            renderview.ViewTime = datadescription.GetTime()

            user_data = datadescription.GetUserData()
            json_string = user_data.GetAbstractArray("metadata").GetVariantValue(0).ToString()
            build_info = json.loads(json_string)
            insitu_script_arguments = build_info["insitu_script_arguments"].split(" ")
            global is_insitu_pipeline
            is_insitu_pipeline = True

            # simulation objects
            for so_info in build_info['sim_objects']:
                data = coprocessor.CreateProducer(datadescription, so_info['name'])
                ProcessSimulationObject(so_info, data, renderview)
            # extracellular substances
            for substance_info in build_info['extracellular_substances']:
                producer = coprocessor.CreateProducer(datadescription, substance_info['name'])
                grid = datadescription.GetInputDescriptionByName(substance_info['name']).GetGrid()
                producer.GetClientSideObject().SetOutput(grid)
                producer.UpdatePipeline()
                ProcessExtracellularSubstance(substance_info, producer, renderview)

            # ------------------------------------------------------------------
            # end default pipeline - start custom script
            # check if a custom script was defined
            try:
                ExtendDefaultPipeline
            except NameError:
                pass
            else:
                ExtendDefaultPipeline(renderview, coprocessor, datadescription, 
                        insitu_script_arguments)

        return Pipeline()

    class CoProcessor(coprocessing.CoProcessor):
        def CreatePipeline(self, datadescription):
            self.Pipeline = _CreatePipeline(self, datadescription)

    coprocessor = CoProcessor()
    return coprocessor

#-------------------------------------------------------------------------------
#Global variables that will hold the pipeline for each timestep
#Creating the CoProcessor object, doesn't actually create the ParaView pipeline.
#It will be automatically setup when coprocessor.UpdateProducers() is called the
#first time.
coprocessor = CreateCoProcessor()

#-------------------------------------------------------------------------------
#Enable Live - Visualizaton with ParaView
coprocessor.EnableLiveVisualization(True, 1)

#------------------------------- Data Selection method -------------------------

def RequestDataDescription(datadescription):
    global coprocessor

    if datadescription.GetForceOutput() == True:
        #We are just going to request all fields and meshes from the simulation code / adaptor.
        for i in range(datadescription.GetNumberOfInputDescriptions()):
            datadescription.GetInputDescription(i).AllFieldsOn()
            datadescription.GetInputDescription(i).GenerateMeshOn()
        return

    #setup requests for all inputs based on the requirements of the pipeline.
    coprocessor.LoadRequestedData(datadescription)

#--------------------------------- Processing method ---------------------------
def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"
    global coprocessor

    #Update the coprocessor by providing it the newly generated simulation data.
    #If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)
    coprocessor.WriteImages(datadescription)

    #Live Visualization, if enabled.
    coprocessor.DoLiveVisualization(datadescription, "localhost", 22222)
