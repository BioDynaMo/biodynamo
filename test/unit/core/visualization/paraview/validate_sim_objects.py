import argparse
import sys
from paraview.simple import *

parser = argparse.ArgumentParser(description='Validate Simulation Objects')
parser.add_argument('--sim_name', action='store', type=str)
parser.add_argument('--num_elements', action='store', type=int)
params = parser.parse_args()

def ValidateDataMember(params, neurite, data_member_name, offset):
    data = neurite.GetPointData().GetArray(data_member_name)

    if data.GetNumberOfTuples() != params.num_elements:
        sys.exit(1)

    for i in range(0, data.GetNumberOfTuples()):
        components = data.GetNumberOfComponents()  
        if components == 1:
            if i != (int(data.GetValue(i)) - offset): 
                print("ERROR for attribute", data_member_name, "expected:", i + offset, "actual:", int(data.GetValue(i)))
                sys.exit(2)
        else:
            t = data.GetTuple(i)
            for j in range(0, components):
                if i != (int(t[j]) - offset): 
                    print("ERROR for attribute", data_member_name, "expected:", i + offset, "actual:", int(t[j]))
                    print(data_member_name, i, " ", t)
                    sys.exit(2)
            

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

LoadState('output/{0}/{0}.pvsm'.format(params.sim_name))

neurite_source = FindSource('NeuriteElement-data')

neurite = paraview.servermanager.Fetch(neurite_source)

ValidateDataMember(params, neurite, "diameter_", 10)
ValidateDataMember(params, neurite, "mass_location_", 0)
ValidateDataMember(params, neurite, "actual_length_", 10)
ValidateDataMember(params, neurite, "uid_", 0)
ValidateDataMember(params, neurite, "daughter_right_", 0)

