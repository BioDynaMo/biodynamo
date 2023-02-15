---
title: "Diffusion"
date: "2019-01-01"
path: "/docs/userguide/diffusion/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the diffusion page."
toc: true
image: ""
next:
    url:  "/docs/userguide/diffusion/"
    title: "Diffusion"
    description: "This is the diffusion page."
sidebar: "userguide"
keywords:
  -diffusion
  -process
  -condition
---

One of BioDynaMo's built-in biological processes is extracellular diffusion.
It is the process of extracellular substances diffusing through space. The constants
that govern the diffusion process can be set by the user. Let's go through an
example where diffusion plays a role.

### Copy the demo code

`diffusion` is one of many installed demos in BioDynaMo. It can be copied out
with `biodynamo demo`.

```bash
bdm demo diffusion .
```

### Inspect the code

Go into the `diffusion` directory and open the source file `src/diffusion.h` in your favorite editor.
We can note the following things from its content:

#### 1. Substance list

```cpp
enum Substances { kKalium };
```

The extracellular substances that will be used in the simulation are listed in
an `enum` data structure. In this case it is just a single substance. According to our C++
coding style we will prepend the substance's name with the letter "k".

#### 2. Initial model

First, create a BioDynaMo simulation:
```cpp
Simulation simulation(argc, argv);
```

Next up is creating the initial model of our simulation.
We start by defining the substance that cells may secrete

```cpp
ModelInitializer::DefineSubstance(kKalium, "Kalium", 0.4, 0, 25);
```

Next, we have to create an initial set of agents and set their
attributes:

```cpp
  auto construct = [&](const Real3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(30);
    cell->SetMass(1.0);
    cell->AddBehavior(new Chemotaxis("Kalium", 0.5));
    return cell;
  };
  ModelInitializer::Grid3D(2, 100, construct);

   // The cell responsible for secretion
  Cell secreting_cell({50, 50, 50});
  secreting_cell.AddBehavior(new Secretion("Kalium", 4));
  simulation.GetExecutionContext()->AddAgent(&secreting_cell);
```

The `construct` lambda defines the properties of each cell that we create. These can be
physical properties (diameter, mass), but also biological properties and behaviors
(chemotaxis, substance secretion)

This example uses the predefined behaviors `Chemotaxis` and `Secretion` that
will govern the behavior of the agents (i.e. cells).
These two behaviors are included by default in BioDynaMo.

One of the cells (the cell at position `{50, 50, 50}`) will be the one secreting the substance;
it therefore gets assigned the `Secretion` behavior.
All other cells are assigned the `Chemotaxis` behavior. 
Basically it makes cells move according to the gradient,
caused by a spatial concentration difference of the substance. 


### Simulation Parameters

Create a `bdm.toml` file in the `diffusion` directory, and copy the following lines
into it:

```
[visualization]
export = true
interval = 10

	[[visualize_agent]]
	name = "Cell"
	additional_data_members = [ "diameter_" ]

	[[visualize_diffusion]]
	name = "Kalium"
	gradient = true

```

This will enable exporting visualization files, so that we can visualize the
simulation after it has finished. Furthermore, we enable the output of the diameter
of our agents (by default named "Cell"), and the gradient data of the
extracellular diffusion

### Build and run the simulation

Run the following commands to build and run the simulation.

``` bash
bdm run
```

### Visualize the simulation

Load the generated ParaView state file as described in [Section Visualization](/docs/userguide/visualization/#export-visualization-files).

From "View", select "Animation Panel". This will display some animation settings
at the bottom of the screen. From the "Mode" select "Real Time".
Then click the Play button at the top of the screen to run the simulation visualization.

<video width="100%" controls>
  <source src="https://cernbox.cern.ch/index.php/s/rzl2Kb4uxny4ZXF/download?path=%2F&files=exported_visualization.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Boundary conditions

For a numerical solution, we need to specify the equation, the boundary
conditions, and the domain. BioDynaMo supports Neumann and Dirichlet boundaries 
with constant coefficients on a cube domain. For instance, you may specify 
no-flux boundaries (homogeneous Neumann) via
``` cpp
ModelInitializer::AddBoundaryConditions(
    kKalium, BoundaryConditionType::kNeumann,
    std::make_unique<ConstantBoundaryCondition>(0));
```
or some Dirichlet boundaries via
``` cpp
ModelInitializer::AddBoundaryConditions(
    kKalium, BoundaryConditionType::kDirichlet,
    std::make_unique<ConstantBoundaryCondition>(1.0));
```

The `ModelInitializer` conveniently wraps the access to the `ResourceManager`
and sets the boundaries. You can also set the boundaries directly by calling
member functions of the `DiffusionGrid` (see API documentation).

If you want to implement more sophisticated boundaries (e.g. with spatial 
dependence), you may derive classes from `BoundaryCondition` and implement its
member `BoundaryCondition::Evaluate(real_t,real_t,real_t,real_t)` accordingly.
If your application required different equations, different domains, different
numerical schemes, please consult the API for `Continuum`, `ScalarField`, and
`VectorField` to see how to interface continuum models with the BioDyanMo
simulation runtime. (see also `bdm demo analytic_continuum`)

### Diffusion parameter constraints
The partial differential equations that describe diffusion are solved 
numerically. This is done using a forward in time and central in space finite difference method. 
As shown in the figure below. The upper indices label the discretization in 
time, and the lower indices describe the discretization in space. The delta 
parameters `t` and `h` denote length in time and space, 
respectively. 

[![Central Difference Method](images/diffusion_central_difference_method.png)](/docs/userguide/diffusion/#diffusion-parameter-constraints)

The diffusion coefficient `D` models the speed of the diffusion process through 
space, while constant `mu` controls the speed at which substances 
decay. The method is not unconditionally stable, thus we 
impose the following constraint on parameters:

[![Parameter Constraint](images/diffusion_parameters_constraint.png)](/docs/userguide/diffusion/#diffusion-parameter-constraints)

Since as a user, you are giving the resolution of the diffusion grid and not the
distance between the grid points, you can determine this value by dividing the
longest dimension of your space by the resolution, or by calling the corresponding
function `DiffusionGrid::GetBoxLength()`.

For more information on the inner workings of the diffusion behavior, please
refer to: https://repository.tudelft.nl/islandora/object/uuid%3A2fa2203b-ca26-4aa2-9861-1a4352391e09?collection=education

### Runge-Kutta method:
We have additionally implemented the 2nd order Runge-Kutta method within BioDynaMo.
The Runge-Kutta method is an iterative method for solving ordinary differential equations (ODEs), both implicitly and explicitly. Often outperforming the Euler method for complex ODEs. 
Unlike the Euler method which estimates the next time step based on the rate of change of the defined ODE at the current point, the Runge-Kutta method is a family of schemes which
involve slope calculations between the current and next time step, with the number of slope estimates depending on the order of the Runge-Kutta method being utilised.

In the case of the 2nd order Runge-Kutta method implemented here, a slope estimate is taken at the midpoint between the current and next time step, in addition to utilising the rate of change
at the current time step.

The Runge-Kutta method solves ODEs of the form:

[![Runge-Kutta function](images/runge_kutta_function.png)](/docs/userguide/diffusion/#runge_kutta_function)

We estimate a solution explicitly using the following steps:

[![Runge-Kutta equation](images/runge_kutta_equation.png)](/docs/userguide/diffusion/#runge_kutta_equation)

Here k1 is the slope at the beginning of the interval and k2 is the slope at the midpoint of the interval. With h determining interval length being solved for.

For example, with a h value of 1, we would be estimating between the current time step t to t+1 with a single midpoint slope estimate. For most ODEs
increasing the number of intervals to be taken per time step increases overall accuracy, as we will be taking a greater number of midpoint slope estimates between t to t+1.

So, if we instead set h = 0.5, the method will first estimate over t to t + 0.5 with a midpoint slope estimate, repeating this for t + 0.5 to t + 1 with a second midpoint slope estimate. 
However, there is a trade-off taken here with increased computational time for increasing accuracy.

As stated, the higher order Runge-Kutta methods often outperform lower order solvers for ODEs, however this does not happen for the currently implemented version of chemical diffusion. 
This is due to the fact that when one breaks down the partial differential equations (PDEs) that define chemical diffusion into the set of ODEs that define equation 3.1, 
the time dependent variable is lost on the right hand side of the equation.
This results in the extra steps being taken for the Runge-Kutta method having minimal impact as there is no time dependent variable to estimate the slope from.

Within BioDynaMo the number of intervals for the Runge-Kutta method to iterate over per time step can be set within the declaration of a diffusion grid itself as follows:

```

DiffusionGrid* dgrid = new RungeKuttaGrid(substance_id, "substance_name", diffusion_coefficient, resolution)
                
```

To access the Runge-Kutta method for diffusion, one simply needs to update the bdm.toml file as follows :

```

[simulation]
diffusion_method = "runge-kutta"

[visualization]
export = true
interval = 10
diffusion_method = "runge-kutta"

	[[visualize_agent]]
	name = "Cell"
	additional_data_members = [ "diameter_" ]

	[[visualize_diffusion]]
	name = "Kalium"
	gradient = true
```



Note:
* This method requires the input of known initial boundary conditions for the ODE being solved.
* This method is also commonly referred to as the midpoint method or improved Euler.	
* This method can additionally be used to solve partial differential equations (PDEs) but requires each component to be individually broken down into separate ODEs.


