#Soma CLustering Example
Let's take a look at a more realistic example called soma clustering. In this
example, we create two types of cells. Each type of cell secretes a specific substance,
and moves along the gradient of its corresponding substance. This will form clusters
of cells that are of the same type.

### Copy out the demo code

Soma clustering is one of many installed demos. It can be copied to a target directory:

```bash
biodynamo demo soma_clustering .
```

### Inspect the code

Go into the `soma_clustering` directory and open the source file `src/soma_clustering.h` in your favorite editor.
We can note the following things from its content:

#### 1. Creating a custom simulation object

In `src/my_cell.h` we can find the following code:

```cpp
class MyCell : public Cell {
  BDM_SIM_OBJECT_HEADER(MyCell, Cell, 1, cell_type_);

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
a type to a cell.

#### 2. Define substances and biology modules

In `src/soma_clustering_biology_modules.h` we can find the listing of the two substances
that are used in this simulation:

```cpp
enum Substances { kSubstance_0, kSubstance_1 };
```

We can also find the biology modules that were used in the Diffusion exercise.

### Configure the simulation

Create a `bdm.toml` file in the `diffusion` directory, and create the following
configuration file:

```
[visualization]
export = true
export_interval = 10

	[[visualize_sim_object]]
	name = "MyCell"
	additional_data_members = [ "diameter_", "cell_type_" ]

	[[visualize_diffusion]]
	name = "Substance_0"

  [[visualize_diffusion]]
  name = "Substance_1"

```

This will enable exporting visualization files, so that we can visualize the
simulation after it has finished. Furthermore, we enable the output of the diameter
and the cell type of our simulation objects (named "MyCell"), and the two substances
that are secreted.

### Build and run the simulation

Run the following commands to build and run the simulation (do not forget to
`biodynamo source` if you haven't already in your terminal):

```bash
biodynamo build
biodynamo run
```

### Visualize the simulation

Open ParaView and navigate to the `diffusion` directory. Open the `cells_data_*`
and `Kalium_*` files as Group (see the Visualization exercise as a reference).

Click on the `cells_data_` entry in the Pipeline Browser. From "Filters", select
"Search" (or do Ctrl + Space). Search for the "Glyph" filter, Apply it, and set
the following properties:

```Python
Glyph Type 		= 'Sphere'
Scalars 		  = 'Diameters'
Scale Mode 		= 'Scalar'
Scale Factor 	= 1
Glyph Mode 		= 'All Points'
```

And hit Apply. You might need to 'Zoom to Fit'. Hit the Play button on the top of
the interface to play the simulation over time.
