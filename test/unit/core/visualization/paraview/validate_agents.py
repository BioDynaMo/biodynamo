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

import argparse
import os
import sys
from paraview.simple import *
from paraview import coprocessing

validation_error = False

def ValidateDataMember(params, neurite, data_member_name, offset):
    data = neurite.GetPointData().GetArray(data_member_name)

    global validation_error 
    if data.GetNumberOfTuples() != params.num_elements:
        print("ERROR Wrong number of agents. Expected:", params.num_elements, 
                "Actual:", data.GetNumberOfTuples())
        validation_error = True
        return

    for i in range(0, data.GetNumberOfTuples()):
        components = data.GetNumberOfComponents()  
        t = data.GetTuple(i)
        for j in range(0, components):
            if i != (int(t[j]) - offset): 
                print("ERROR for attribute", data_member_name, "expected:", i + offset, 
                        "actual:", int(t[j]))
                validation_error = True
                return

# Returning an error code using sys.exit does not work for the insitu tests
# because it would exit the whole unit test process.
# Thus we create a valid file to indicate a passing test
def CreateValidFile(sim_name):
    with open("output/{0}/valid".format(sim_name), 'w') as fp: 
        pass

# Entry point for insitu visualization
def ExtendDefaultPipeline(renderview, coprocessor, datadescription, script_args):
    parser = argparse.ArgumentParser(description='Validate Agents')
    parser.add_argument('--sim_name', action='store', type=str)
    parser.add_argument('--num_elements', action='store', type=int)
    params, other = parser.parse_known_args(script_args)
    
    neurite_source = FindSource('NeuriteElement-data')
    
    neurite = paraview.servermanager.Fetch(neurite_source)
    
    ValidateDataMember(params, neurite, "diameter_", 10)
    ValidateDataMember(params, neurite, "mass_location_", 0)
    ValidateDataMember(params, neurite, "actual_length_", 10)
    ValidateDataMember(params, neurite, "uid_", 0)
    ValidateDataMember(params, neurite, "daughter_right_", 0)

    global validation_error
    if not validation_error:
        CreateValidFile(params.sim_name)

# Entry point for export visualization
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Validate Agents')
    parser.add_argument('--sim_name', action='store', type=str)
    parser.add_argument('--use_pvsm', action='store_true', dest="use_pvsm")
    params, args = parser.parse_known_args()
   
    #### disable automatic camera reset on 'Show'
    paraview.simple._DisableFirstRenderCameraReset()
    if params.use_pvsm:
        LoadState('output/{0}/{0}.pvsm'.format(params.sim_name))
    else:
        sys.path.insert(0, "{0}/include/core/visualization/paraview".format(os.environ['BDMSYS']))
        from generate_pv_state import BuildDefaultPipeline
        BuildDefaultPipeline('output/{0}/simulation_info.json'.format(params.sim_name))
    ExtendDefaultPipeline(None, None, None, sys.argv[1:]) 

