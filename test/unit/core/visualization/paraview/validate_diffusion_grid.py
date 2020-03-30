import argparse
import sys
from paraview.simple import *

parser = argparse.ArgumentParser(description='Validate Diffusion Grid')
parser.add_argument('--sim_name', action='store', type=str)
parser.add_argument('--num_elements', action='store', type=int)
params = parser.parse_args()

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

LoadState('output/{0}/{0}.pvsm'.format(params.sim_name))

substance_source = FindSource('Substance-concentration')

substance = paraview.servermanager.Fetch(substance_source)
data = substance.GetPointData().GetArray('Substance Concentration')

if data.GetNumberOfTuples() != params.num_elements:
    sys.exit(1)

for i in range(0, data.GetNumberOfTuples()):
    if i != int(data.GetValue(i)): 
        sys.exit(2)
