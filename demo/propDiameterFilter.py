# Populate idx_values with (idx, value) tuples
attribute = 'Diameter'
idx_values = [ ]

### DO NOT EDIT FROM HERE ###
from paraview import vtk

pdo = self.GetOutput()
pdi = self.GetPolyDataInput()
nPoints = pdi.GetNumberOfPoints()

for arrayName in [ attribute ]:
    oldArray = pdi.GetPointData().GetArray(arrayName)
    if not oldArray:
        print 'Array "%s" does not exists!' % arrayName
        continue

    vtkNewArray = vtk.vtkDoubleArray()
    vtkNewArray.DeepCopy(oldArray)
    vtkNewArray.SetName("Prop" + arrayName)

    # Update values
    for idx, val in idx_values:
        vtkNewArray.SetValue(idx, val)

    # Set new array
    pdo.GetPointData().AddArray(vtkNewArray)

################################
