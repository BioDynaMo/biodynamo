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
import sys
import os
from paraview.simple import *
from paraview import coprocessing

# Returning an error code using sys.exit does not work for the insitu tests
# because it would exit the whole unit test process.
# Thus we create a file validation_error to indicate a failed validation
def CreateValidFile(sim_name):
    with open("output/{0}/valid".format(sim_name), 'w') as fp: 
        pass

# Entry point for insitu visualization
def ExtendDefaultPipeline(renderview, coprocessor, datadescription, script_args):
    parser = argparse.ArgumentParser(description='Validate Diffusion Grid')
    parser.add_argument('--sim_name', action='store', type=str)
    parser.add_argument('--num_elements', action='store', type=int)
    params = parser.parse_args(script_args)
    
    substance_source = FindSource('Substance-concentration')
    
    substance = paraview.servermanager.Fetch(substance_source)
    data = substance.GetPointData().GetArray('Substance Concentration')
    
    if data.GetNumberOfTuples() != params.num_elements:
        print("ERROR number of diffusion grid elements wrong: expected:", param.num_elements, 
                "actual:", data.GetNumberOfTuples())
        return
    
    for i in range(0, data.GetNumberOfTuples()):
        if i != int(data.GetValue(i)): 
            print("ERROR diffusion grid element", i, " has wrong value: expected:", i, 
                    "actual:", data.GetValue(i))
            return
    
    CreateValidFile(params.sim_name) 

# Entry point for export visualization
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Validate Diffusion Grid')
    parser.add_argument('--sim_name', action='store', type=str)
    params, args = parser.parse_known_args()
    
    #### disable automatic camera reset on 'Show'
    paraview.simple._DisableFirstRenderCameraReset()
    LoadState('output/{0}/{0}.pvsm'.format(params.sim_name))
    ExtendDefaultPipeline(None, None, None, sys.argv[1:]) 

