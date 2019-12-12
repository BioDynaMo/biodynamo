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

            renderView1 = GetActiveViewOrCreate('RenderView')

            #create a producer from a simulation input
            # diffusion_grid_data = coprocessor.CreateProducer(datadescription, 'dgrid_data')
            for i in range(datadescription.GetNumberOfInputDescriptions()):
                data_name = datadescription.GetInputDescriptionName(i)
                data = coprocessor.CreateProducer(datadescription, data_name)
                grid = datadescription.GetInputDescription(i).GetGrid()

                #create a new 'Glyph'
                if grid.IsA("vtkUnstructuredGrid") == True:
                    gui_name = data_name + "_glyph"
                    glyph = Glyph(guiName=gui_name, Input=data, GlyphType='Sphere')
                    glyph.Scalars = [ 'POINTS', 'Diameters' ]
                    glyph.Vectors = [ 'POINTS', 'None' ]
                    glyph.ScaleMode = 'scalar'
                    glyph.GlyphMode = 'All Points'
                    glyph.GlyphTransform = 'Transform2'

        return Pipeline()

    class CoProcessor(coprocessing.CoProcessor):
        def CreatePipeline(self, datadescription):
            self.Pipeline = _CreatePipeline(self, datadescription)

    coprocessor = CoProcessor()
    freqs = { 'input' : [100, 100]}
    coprocessor.SetUpdateFrequencies(freqs)
    return coprocessor

#---------------------------------------------------------------------------------------
#Global variables that will hold the pipeline for each timestep
#Creating the CoProcessor object, doesn't actually create the ParaView pipeline.
#It will be automatically setup when coprocessor.UpdateProducers() is called the
#first time.
coprocessor = CreateCoProcessor()

#---------------------------------------------------------------------------------------
#Enable Live - Visualizaton with ParaView
coprocessor.EnableLiveVisualization(True, 1)

#------------------------------- Data Selection method ---------------------------------

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

#--------------------------------- Processing method ----------------------------------

def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"
    global coprocessor

    #Update the coprocessor by providing it the newly generated simulation data.
    #If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)

    #Live Visualization, if enabled.
    coprocessor.DoLiveVisualization(datadescription, "localhost", 22222)
