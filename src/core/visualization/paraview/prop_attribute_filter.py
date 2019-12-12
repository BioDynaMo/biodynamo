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

# Populate idx_vals_tuples with (idx, value) tuples
attribute = 'Diameter'
idx_vals_tuples = [ ]

### DO NOT EDIT FROM HERE ###
from paraview import vtk

pdo = self.GetOutput()
pdi = self.GetPolyDataInput()
nPoints = pdi.GetNumberOfPoints()

for arrayName in [ attribute ]:
    oldArray = pdi.GetPointData().GetArray(arrayName)
    if not oldArray:
        print 'Attribute "%s" does not exists!' % arrayName
        continue

    valid_tuples = [ ]
    for idx, val in idx_vals_tuples:
        if idx < 0 or idx >= nPoints:
            print 'Skipping invalid tuples (%d, %lf)' % (idx, val)
            continue

        valid_tuples.append((idx, val))

    if len(valid_tuples) == 0:
        continue

    vtkPropIdx = vtk.vtkIdTypeArray()
    vtkPropIdx.SetName("PropIdx" + arrayName)
    vtkPropIdx.SetNumberOfValues( len(valid_tuples) )

    vtkPropVals = vtk.vtkDoubleArray()
    vtkPropVals.SetName("PropVals" + arrayName)
    vtkPropVals.SetNumberOfValues( len(valid_tuples) )

    # Record changes
    for i, (idx, val) in enumerate(valid_tuples):
        vtkPropIdx.SetValue(i, idx)
        vtkPropVals.SetValue(i, val)

    # Set new array
    pdo.GetFieldData().AddArray(vtkPropIdx)
    pdo.GetFieldData().AddArray(vtkPropVals)

################################
