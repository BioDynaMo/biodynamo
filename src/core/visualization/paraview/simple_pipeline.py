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

#Code generated from cpstate.py to create the CoProcessor.
#ParaView 5.3.0 64 bits

#CoProcessor definition
def CreateCoProcessor():
    def _CreatePipeline(coprocessor, datadescription):
        class Pipeline :
            #disable automatic camera reset on 'Show'
            paraview.simple._DisableFirstRenderCameraReset()

            renderView1 = CreateView('RenderView')
            renderView1.ViewTime = datadescription.GetTime()

            # #create a producer from a simulation input
            # diffusion_grid_data = coprocessor.CreateProducer(datadescription, 'dgrid_data')
            print(datadescription.GetNumberOfInputDescriptions())
            for i in range(datadescription.GetNumberOfInputDescriptions()):
                data_name = datadescription.GetInputDescriptionName(i)
                data = coprocessor.CreateProducer(datadescription, data_name)
                grid = datadescription.GetInputDescription(i).GetGrid()

                # print(data_name)
                # print(data)
                # print(grid)

                #create a new 'Glyph'
                if grid.IsA("vtkUnstructuredGrid") == True:
                    glyph1 = Glyph(guiName="Cells", Input=data)
                    glyph1.GlyphTransform = 'Transform2'

                    glyph1.GlyphType = 'Sphere'
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
                    glyph1.ScaleArray = ['POINTS', "diameter_"]

                    renderView1.Update()

            # ------------------------------------------------------------------
            # end default pipeline - start custom script

            renderView1 = GetActiveViewOrCreate('RenderView')
            renderView1.Update()
            cells = FindSource('Cells')
            SetActiveSource(cells)
            renderView1.ResetCamera()

            cells.GlyphType.ThetaResolution = 20
            cells.GlyphType.PhiResolution = 20

            # Properties modified on renderView1
            # renderView1.EnableOSPRay = 1
            #
            # # get the material library
            # materialLibrary1 = GetMaterialLibrary()
            #
            # # Properties modified on renderView1
            # renderView1.OSPRayRenderer = 'pathtracer'
            #
            # # update the view to ensure updated data information
            # renderView1.Update()
            #
            # # Properties modified on renderView1
            # renderView1.Shadows = 1
            #
            # # Properties modified on renderView1
            # renderView1.SamplesPerPixel = 20

            # SaveScreenshot("my-screen-{}.png".format(datadescription.GetTimeStep()), renderView1, ImageResolution=[512, 512], FontScaling='Scale fonts proportionally')
            coprocessor.RegisterView(renderView1,
                               filename="my-screen-%t.png", freq=1, fittoscreen=1, magnification=1, width=512, height=512)


        return Pipeline()

    class CoProcessor(coprocessing.CoProcessor):
        def CreatePipeline(self, datadescription):
            self.Pipeline = _CreatePipeline(self, datadescription)

    coprocessor = CoProcessor()
    # freqs = { 'Cells' : [1]}
    # coprocessor.SetUpdateFrequencies(freqs)
    return coprocessor

#---------------------------------------------------------------------------------------
#Global variables that will hold the pipeline for each timestep
#Creating the CoProcessor object, doesn't actually create the ParaView pipeline.
#It will be automatically setup when coprocessor.UpdateProducers() is called the
#first time.
coprocessor = CreateCoProcessor()

#---------------------------------------------------------------------------------------
#Enable Live - Visualizaton with ParaView
# coprocessor.EnableLiveVisualization(True, 1)

#------------------------------- Data Selection method ---------------------------------

def RequestDataDescription(datadescription):
    global coprocessor

    print("Python Catalyst RequestDataDescription")
    # print(datadescription)

    if datadescription.GetForceOutput() == True:
        print("   datadescription.GetForceOutput()")

        #We are just going to request all fields and meshes from the simulation code / adaptor.
        print("   {}".format(datadescription.GetNumberOfInputDescriptions()))
        for i in range(datadescription.GetNumberOfInputDescriptions()):
            print("   {0}".format(datadescription.GetInputDescription(i)))
            datadescription.GetInputDescription(i).AllFieldsOn()
            datadescription.GetInputDescription(i).GenerateMeshOn()
        return

    #setup requests for all inputs based on the requirements of the pipeline.
    coprocessor.LoadRequestedData(datadescription)

#--------------------------------- Processing method ----------------------------------
def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"
    global coprocessor

    print("Python Catalyst DoCoProcessing")
    # print(datadescription)
    print(datadescription.GetTime())
    print(datadescription.GetTimeStep())

    #Update the coprocessor by providing it the newly generated simulation data.
    #If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)
    coprocessor.WriteImages(datadescription)

    #Live Visualization, if enabled.
    # coprocessor.DoLiveVisualization(datadescription, "localhost", 22222)
