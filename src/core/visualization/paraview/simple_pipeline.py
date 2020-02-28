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
            print("FOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOo")
            #disable automatic camera reset on 'Show'
            paraview.simple._DisableFirstRenderCameraReset()

            # renderView1 = GetActiveViewOrCreate('RenderView')
            #
            # glyph1 = Glyph(Input=so_data, GlyphType=glyph_type)
            # glyph1.GlyphTransform = 'Transform2'
            #
            # glyph1.GlyphType = glyph_type
            # glyph1.ScaleFactor = 1.0
            # glyph1.GlyphMode = 'All Points'
            # #
            # # # show data in view
            # glyph1Display = Show(glyph1, render_view)
            # # # trace defaults for the display properties.
            # glyph1Display.Representation = 'Surface'
            # glyph1Display.ColorArrayName = [None, '']
            # glyph1Display.OSPRayScaleFunction = 'PiecewiseFunction'
            # glyph1Display.SelectOrientationVectors = 'None'
            # glyph1Display.ScaleFactor = 1.0
            # glyph1Display.SelectScaleArray = 'None'
            # glyph1Display.GlyphType = 'Sphere'
            # glyph1Display.GlyphTableIndexArray = 'None'
            # glyph1Display.DataAxesGrid = 'GridAxesRepresentation'
            # glyph1Display.PolarAxes = 'PolarAxesRepresentation'
            # glyph1Display.GaussianRadius = -1.0000000000000001e+298
            # glyph1Display.SetScaleArray = [None, '']
            # glyph1Display.ScaleTransferFunction = 'PiecewiseFunction'
            # glyph1Display.OpacityArray = [None, '']
            # glyph1Display.OpacityTransferFunction = 'PiecewiseFunction'
            #
            # renderView1.Update()


            # #create a producer from a simulation input
            # diffusion_grid_data = coprocessor.CreateProducer(datadescription, 'dgrid_data')
            print(datadescription.GetNumberOfInputDescriptions())
            for i in range(datadescription.GetNumberOfInputDescriptions()):
                data_name = datadescription.GetInputDescriptionName(i)
                data = coprocessor.CreateProducer(datadescription, data_name)
                grid = datadescription.GetInputDescription(i).GetGrid()

                print(data_name)
                print(data)
                print(grid)
            #
            #     #create a new 'Glyph'
            #     if grid.IsA("vtkUnstructuredGrid") == True:
            #         gui_name = data_name + "_glyph"
            #         glyph = Glyph(guiName=gui_name, Input=data, GlyphType='Sphere')
            #         glyph.Scalars = [ 'POINTS', 'Diameters' ]
            #         glyph.Vectors = [ 'POINTS', 'None' ]
            #         glyph.ScaleMode = 'scalar'
            #         glyph.GlyphMode = 'All Points'
            #         glyph.GlyphTransform = 'Transform2'

        return Pipeline()

    class CoProcessor(coprocessing.CoProcessor):
        def CreatePipeline(self, datadescription):
            self.Pipeline = _CreatePipeline(self, datadescription)

    coprocessor = CoProcessor()
    # freqs = { 'input' : [100, 100]}
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
    print(datadescription)

    #Update the coprocessor by providing it the newly generated simulation data.
    #If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)

    #Live Visualization, if enabled.
    # coprocessor.DoLiveVisualization(datadescription, "localhost", 22222)
