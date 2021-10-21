---
title: "Soma Clustering"
date: "2020-08-01"
path: "/docs/userguide/soma_clustering/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the soma clustering page."
toc: true
image: ""
next:
    url:  "/docs/userguide/soma_clustering/"
    title: "Soma Clustering"
    description: "This is the soma clustering page."
sidebar: "userguide"
keywords:
  -soma
  -clustering
  -biology
  -code
  -example
  -demo
---

Let's take a look at a more realistic example called soma clustering. In this
example, we create two types of cells. Each type of cell secretes a specific substance,
and moves along the gradient of its corresponding substance. This will form clusters
of cells that are of the same type.

## Copy out the demo code

Soma clustering is one of many installed demos. It can be copied to a target directory:

```bash
biodynamo demo soma_clustering .
```

## Inspect the code

Firstly, change to the new `soma_clustering` directory that you have just copied.
We can note the following things from its content:

### 1. Creating a custom agent

Begin by opening `src/my_cell.h` in your favourite text editor. We can find the following code:

```cpp
class MyCell : public Cell {
  BDM_AGENT_HEADER(MyCell, Cell, 1);

 public:
  MyCell() {}
  MyCell(const Double3& position) : Base(position) {}

  void SetCellType(int t) { cell_type_ = t; }
  int GetCellType() const { return cell_type_; }

 private:
  int cell_type_;
```

We create a new type of cell called "MyCell" that extends the default Cell.
It contains a new data member called `cell_type_` that makes it possible to assign
an integer value of type to a cell.

### 2. Define substances and behaviors

In `src/soma_clustering.h` we can find the set-up for the soma clustering simulation.
We begin by listing of the two substances that are used in this simulation:

```cpp
enum Substances { kSubstance_0, kSubstance_1 };
```
The properties of these substances are later defined, with their substance_name,
diffusion_coefficient, decay_constant, resolution.

```cpp
ModelInitializer::DefineSubstance(kSubstance0, "Substance_0", 0.5, 0.1, 20);
ModelInitializer::DefineSubstance(kSubstance1, "Substance_1", 0.5, 0.1, 20);
```
We can also find the behaviors that were used in the Diffusion exercise.
These are added when constructing the agents in the simulation:

```cpp
cell->AddBehavior(new Secretion(substance_name));
cell->AddBehavior(new Chemotaxis(substance_name, 5));
```
The initial number of cells is set via the ```int num_cells``` variable (set to 20,000).
The environment (simulation space) settings are also set to create a cube of 250x250x250.

```cpp
auto set_param = [](Param* param) {
  // Create an artificial bound for the simulation space
  param->bound_space = Param::BoundSpaceMode::kClosed;
  param->min_bound = 0;
  param->max_bound = 250;
  param->unschedule_default_operations = {"mechanical forces"};
};
```


## Configure the simulation

Create a `bdm.toml` file in the `soma_clustering` directory, and create the following
configuration file:

```
[visualization]
export = true
interval = 10

	[[visualize_agent]]
	name = "MyCell"
	additional_data_members = [ "diameter_", "cell_type_" ]

	[[visualize_diffusion]]
	name = "Substance_0"

  [[visualize_diffusion]]
  name = "Substance_1"

```

This will enable exporting visualization files, so that we can visualize the
simulation after it has finished. Furthermore, we enable the output of the diameter
and the cell type of our agents (named "MyCell"), and the two substances
that are secreted ("Substance_0" and "Substance_1"). 
We may also add the diffusion gradient by adding the setting `gradient = true`.

## Build and run the simulation

Run the following commands to build and run the simulation (do not forget to
`source` BioDynaMo if you haven't already in your terminal, i.e. `source <installation-directory>/bin/thisbdm.sh`):

```bash
biodynamo build
biodynamo run
```

## Visualize the simulation

To visualize the simulation, consult the Visualization tutorial (https://biodynamo.org/docs/userguide/visualization/).
Open ParaView (`paraview &`) and go to File -> Load State. Locate the soma_clustering directory and select the `soma_clustering.pvsm` file within output directory.
In the next window keep the default ('Use File Names From State').

Now that simulation can be visualized by hitting the Play button on the top of the interface.
This will play the simulation over time.

By clicking on `MyCells` in the Pipeline Browser, the Properties window will be shown.
Under 'Coloring', click the drop-down to change 'Solid Color' to 'cell_type_' to view cells of each type using a different colour.

To ensure best results, in the Properties window for "MyCells", search "Glyph" and set the following properties:
  ```Python
Glyph Type 		= 'Sphere'
Scalars 		  = 'Diameters'
Scale Mode 		= 'Scalar'
Scale Factor 	= 1
Glyph Mode 		= 'All Points'
```
Don't forget to press "Apply" in the Properties window after making changes and pressing "Zoom to Fit" if required.
A visualization of soma clustering using BioDynaMo can also be found here: https://youtu.be/jlOk_Y3SUHo
