from paraview.simple import *
from paraview import vtk
from paraview import coprocessing

#Code generated from cpstate.py to create the CoProcessor.
#ParaView 5.3.0 64 bits

# the frequency to output everything
outputfrequency = 1
# flag to write to file or not
write_to_file = False

#CoProcessor definition
def CreateCoProcessor():
    def _CreatePipeline(coprocessor, datadescription):
        class Pipeline :
            #state file generated using paraview version 5.3.0

            #setup the data processing pipelines

            #disable automatic camera reset on 'Show'
            paraview.simple._DisableFirstRenderCameraReset()

            #create a producer from a simulation input
            # diffusion_grid_data = coprocessor.CreateProducer(datadescription, 'dgrid_data')
            cells_data = coprocessor.CreateProducer(datadescription, 'cells_data')

            #create a new 'Glyph'
            #glyph1 = Glyph(Input = propDiameterFilter, GlyphType = 'Sphere')
            glyph1 = Glyph(guiName= 'Cells', Input= cells_data, GlyphType = 'Sphere')
            glyph1.Scalars = [ 'POINTS', 'Cell Diameters' ]
            glyph1.Vectors = [ 'POINTS', 'None' ]
            glyph1.ScaleMode = 'scalar'
            glyph1.GlyphMode = 'All Points'
            glyph1.GlyphTransform = 'Transform2'

            timestep = datadescription.GetTimeStep()

            if write_to_file == True:
                # diffusion_grid_writer = servermanager.writers.XMLPImageDataWriter(Input=diffusion_grid_data)
                diffusion_cell_Writer = servermanager.writers.XMLPUnstructuredGridWriter(Input=cells_data)

                coprocessor.RegisterWriter(diffusion_grid_writer, "diffusion_grid_%t.pvti", freq=outputfrequency)
                coprocessor.RegisterWriter(diffusion_cell_Writer, "diffusion_cells_%t.pvtu", freq=outputfrequency)

            SetActiveSource(glyph1)

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
    "Callback to populate the request for current timestep"
    timestep = datadescription.GetTimeStep()
    
    # datadescription.GetInputDescriptionByName('dgrid_data').AllFieldsOn()
    datadescription.GetInputDescriptionByName('cells_data').AllFieldsOn()
    # datadescription.GetInputDescriptionByName('dgrid_data').GenerateMeshOn()
    datadescription.GetInputDescriptionByName('cells_data').GenerateMeshOn()
    
    global coprocessor


    if datadescription.GetForceOutput() == True:
        #We are just going to request all fields and meshes from the simulation code / adaptor.
        for i in range(datadescription.GetNumberOfInputDescriptions()):
            datadescription.GetInputDescription(i).AllFieldsOn()
            datadescription.GetInputDescription(i).GenerateMeshOn()
        return

    #setup requests for all inputs based on the requirements of the
    #pipeline.
    coprocessor.LoadRequestedData(datadescription)

#--------------------------------- Processing method ----------------------------------

def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"
    global coprocessor

    #Update the coprocessor by providing it the newly generated simulation data.
    #If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)

    #Write output data, if appropriate.
    coprocessor.WriteData(datadescription);

    #Write image capture(Last arg : rescale lookup table), if appropriate.
    coprocessor.WriteImages(datadescription, rescale_lookuptable = False)

    #Live Visualization, if enabled.
    coprocessor.DoLiveVisualization(datadescription, "localhost", 22222)
