import vtk
import os

steps = 900
ptuFiles = 8
reader = vtk.vtkXMLUnstructuredGridReader()

# first read all locations

planets = ["Mercury", "Venus", "Earth",
        "Mars", "Jupiter", "Saturn", "Uranus",
        "Neptune", "Pluto"]
locations = [[], [], [], [], [], [], [], [], []]
try:
    # Read pvtu file for each time step and extract files
    for i in range(steps):
        k = 0
        for j in range(ptuFiles):

            # Find the output files
            path = f"output/solar_system/Planet-{i}_{j}.vtu"
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
    # How many days/steps will it take for a full rotation
    data = [88.0, 224.7, 365.24, 687.0, 
                331.0, 10.747, 30.589, 
                59800.0, 90560.0]
    stars = "**********************"
    print(f'\n\n{stars}{stars}{stars}\n')
    for planet in planets:
        index = planets.index(planet)
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
                print(f"    The relative error of {planet}'s full rotation is {relativeError*100:.2f}%")
                break
    print(f'\n{stars}{stars}{stars}') 
    print(f"\n\nPrevious runs of this algorithm returned the following output:\n")
    print(f"    The relative error of Mercury's full rotation is 3.41%")
    print(f"    The relative error of Venus's full rotation is 0.31%")
    print(f"    The relative error of Earth's full rotation is 0.07%")
    print(f"    The relative error of Mars's full rotation is 0.44%")
    print(f"\n\n ** The rest of the planets did not complete a full roation")

except Exception as e:
    print(f"error: {e}") 