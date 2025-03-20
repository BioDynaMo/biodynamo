import vtk
import os

steps = 500
ptuFiles = 8
reader = vtk.vtkXMLUnstructuredGridReader()

# first read all locations

satellites = ["Phobos", "Deimos"]
locations = [[], []]
try:
    # Read pvtu file for each time step and extract files
    for i in range(steps):
        k = 0
        for j in range(ptuFiles):

            # Find the output files
            path = f"output/mars/Satellite-{i}_{j}.vtu"
            if not os.path.exists(path):
                raise Exception("Output file not found")
            reader.SetFileName(path)
            reader.Update()
            output = reader.GetOutput()

            # Get all points from each file
            for l in range(output.GetNumberOfPoints()):
                locations[k].append(output.GetPoints().GetPoint(l))
                k = k + 1
    
    # Scientific Data
    # Since each step is 0.1 days
    data = [76.6, 303.5]
    stars = "**********************"
    print(f'\n\n{stars}{stars}{stars}\n')
    for satellite in satellites:
        index = satellites.index(satellite)
        # Find location after 1 full rotation
        for step in range(1,steps):

            A = locations[index][step][1] > 0
            B = locations[index][step-1][1] < 0

            if(A and B):
                # Find the relative error
                # Relative error = |x - x_0| / x
                # where x is actual value
                # and x is the measured value
                relativeError = abs(step - data[index]) / data[index]
                print(f"    The relative error of {satellite}'s full rotation is {relativeError*100:.2f}%")
                break
    print(f'\n{stars}{stars}{stars}') 
    print(f"\n\nPrevious runs of this algorithm returned the following output:\n")
    print(f"    The relative error of Phobo's full rotation is 0.52%")
    print(f"    The relative error of Deimo's full rotation is 0.16%")
    
except Exception as e:
    print(f"error: {e}") 
