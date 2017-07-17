# Cells indexes
cell_ids = [ ]

# Desired cell diameter values
cells_diameter = [ ]

# Currently 'Diameter' is supported
arrayName = 'Diameter'
print(arrayName)

### DO NOT EDIT HERE ###
from paraview import vtk

pdi = self.GetPolyDataInput()
nPoints = pdi.GetNumberOfPoints()

newArray = vtk.vtkDoubleArray()
newArray.SetName("Prop" + arrayName)
oldArray = pdi.GetPointData().GetArray(arrayName)

# Copy array
# TODO: Make that faster! (pointer's stuff)
newArray.SetNumberOfValues(nPoints)
for j in range(nPoints):
    newArray.SetValue(j, oldArray.GetValue(j))

# Change values
for j in range(len(cell_ids)):
    id = cell_ids[j]
    newArray.SetValue(id, cells_diameter[j])

# Set new array
pdo = self.GetOutput()
pdo.GetPointData().AddArray(newArray)

#del cell_ids[:]
#del cells_diameter[:]

