# Cells indexes
cell_ids = [ ]

# Desired cell diameter values
cells_diameter = [ ]

# Currently only 'Diameter' is supported
arrays = [ 'Diameter' ]

### DO NOT EDIT FROM HERE ###
from paraview import vtk

pdo = self.GetOutput()
pdi = self.GetPolyDataInput()
nPoints = pdi.GetNumberOfPoints()

for arrayName in arrays:
    oldArray = pdi.GetPointData().GetArray(arrayName)
    if not oldArray:
        print 'Array "%s" does not exists!' % arrayName
        continue

    vtkNewArray = vtk.vtkDoubleArray()
    vtkNewArray.SetName("Prop" + arrayName)

    # Copy array
    # TODO: Make that faster! (maybe copy)
    vtkNewArray.SetNumberOfValues(nPoints)
    for j in range(nPoints):
        vtkNewArray.SetValue(j, oldArray.GetValue(j))

    # Update values
    for j in range(len(cell_ids)):
        vtkNewArray.SetValue(cell_ids[j], cells_diameter[j])

    # Set new array
    pdo.GetPointData().AddArray(vtkNewArray)

################################
