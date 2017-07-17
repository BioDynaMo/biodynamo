from paraview.simple import *
from paraview import vtk
from paraview import coprocessing

#Code generated from cpstate.py to create the CoProcessor.
#ParaView 5.3.0 64 bits

def readFile(filename):
    with open(filename, 'r') as f:
        return f.read()

#CoProcessor definition
def CreateCoProcessor():
    def _CreatePipeline(coprocessor, datadescription):
        class Pipeline :
            #state file generated using paraview version 5.3.0

            #setup the data processing pipelines

            #disable automatic camera reset on 'Show'
            paraview.simple._DisableFirstRenderCameraReset()

            #create a new 'XML Unstructured Grid Reader'
            #create a producer from a simulation input
            results4Paraview0vtu = coprocessor.CreateProducer(datadescription, 'input')

            print results4Paraview0vtu

            # create new Programmable filter for propagating user changes
            propDiameterFilter = ProgrammableFilter(Input=results4Paraview0vtu)
            propDiameterFilter.CopyArrays = True
            try:
                propDiameterFilter.Script = readFile("../demo/propDiameterFilter.py")
            except IOError as err:
                print 'Error: ' + err.strerror

            #create a new 'Calculator'
            calculator1 = Calculator(Input = propDiameterFilter)
            calculator1.ResultArrayName = 'Radius'
            calculator1.Function = '0.5*Diameter'

            #create a new 'Glyph'
            #glyph1 = Glyph(Input = propDiameterFilter, GlyphType = 'Sphere')
            glyph1 = Glyph(Input = calculator1, GlyphType = 'Sphere')
            glyph1.Scalars = [ 'POINTS', 'Radius' ]
            glyph1.Vectors = [ 'POINTS', 'None' ]
            glyph1.ScaleMode = 'scalar'
            glyph1.GlyphMode = 'All Points'
            glyph1.GlyphTransform = 'Transform2'

            #finally, restore active source
            SetActiveSource(glyph1)

        return Pipeline()

    class CoProcessor(coprocessing.CoProcessor):
        def CreatePipeline(self, datadescription):
            self.Pipeline = _CreatePipeline(self, datadescription)

    coprocessor = CoProcessor()

    #these are the frequencies at which the coprocessor updates.
    freqs = { 'input' : []}
    coprocessor.SetUpdateFrequencies(freqs)
    return coprocessor

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#Global variables that will hold the pipeline for each timestep
#Creating the CoProcessor object, doesn't actually create the ParaView pipeline.
#It will be automatically setup when coprocessor.UpdateProducers() is called the
#first time.
coprocessor = CreateCoProcessor()

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#Enable Live - Visualizaton with ParaView
coprocessor.EnableLiveVisualization(True, 1)

#-- -- -- -- -- -- -- -- -- -- -- Data Selection method -- -- -- -- -- -- -- -- -- -- --

def RequestDataDescription(datadescription):
    "Callback to populate the request for current timestep"
    global coprocessor


    if datadescription.GetForceOutput() == True:
#We are just going to request all fields and meshes from the simulation
#code / adaptor.
        for i in range(datadescription.GetNumberOfInputDescriptions()):
            datadescription.GetInputDescription(i).AllFieldsOn()
            datadescription.GetInputDescription(i).GenerateMeshOn()
        return

#setup requests for all inputs based on the requirements of the
#pipeline.
    coprocessor.LoadRequestedData(datadescription)

#-- -- -- -- -- -- -- -- -- -- -- -- Processing method -- -- -- -- -- -- -- -- -- -- -- --

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

    # ---------------------------------------------------------------------------
    ## Define filter name for data-feedback
    filterName = "ProgrammableFilter1"

    # Fetch filter object
    propFilter = FindSource(filterName)
    if not propFilter:
        print 'Warning: Filter "%s" cannot be found in Paraview filters' % filterName
        return

    try:
        realFilter = servermanager.Fetch(propFilter)
    except Exception as ex:
        print "Error: " + ex.strerror
        return

    if not realFilter:
        print 'Warning: Filter "%s" cannot be fetched from the Paraview client' % filterName
        return

    # Fetch all propagation arrays (ArrayName = "Prop*")
    filterPointData = realFilter.GetPointData()
    propArrays = filter(
        lambda array: array.GetName().startswith("Prop"),
        ( filterPointData.GetArray(i) for i in range(filterPointData.GetNumberOfArrays()) )
    )

    # Debugging
    #for array in propArrays:
    #    resultArray = [ array.GetValue(i) for i in range(array.GetSize()) ]
    #    print '[%d] %s' % (len(resultArray), resultArray)
    # ----------------------------

    # Serialize data
    userData = vtk.vtkFieldData()
    for array in propArrays:
        userData.AddArray(array)

    # Create array of propagation array names
    propArrayNames = vtk.vtkStringArray()
    propArrayNames.SetName("PropArrays")
    propArrayNames.SetNumberOfValues(len(propArrays))
    for j, array in enumerate(propArrays):
        propArrayNames.SetValue(j, array.GetName())

    userData.AddArray(propArrayNames)

    # Send data to simulator
    datadescription.SetUserData(userData)

