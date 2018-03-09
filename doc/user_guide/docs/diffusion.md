One of BioDynaMo's built-in biological processes is extracellular diffusion.
It is the process of extracellular substances diffusing through space. The constants
that govern the diffusion process can be set by the user. Let's go through an
example where diffusion plays a role.

### Download the code from Github

If you are still in the `hello_world` directory.

#### On Linux:

``` sh
biodynamo.git clone https://github.com/BioDynaMo/simulation-templates.git
cd simulation-templates
biodynamo.git checkout newcastle
```

#### On Mac OS:

``` sh
git clone https://github.com/BioDynaMo/simulation-templates.git
cd simulation-templates
git checkout newcastle
```

### Inspect the code

Go into the `diffusion` directory and open the source file `src/diffusion_biology_modules.h` in your favorite editor.
We can note the following things from its content:

#### 1. List the substances

``` C++
enum Substances { kKalium };
```

The extracellular substances that will be used in the simulation are listed in
an `enum` data structure. In this case it is just a single substance. According to our C++
coding style we will prepend the substance's name with the letter "k".

#### 2. Set up the biology modules

Open the `src/diffusion.h` source file.

In order for BioDynaMo to anticipate the biology modules that you want to use in
the simulation, we need to declare them in our `CompileTimeParameters` as such:

``` C++
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<Chemotaxis, KaliumSecretion>;
};
```

The important part here is the `Chemotaxis` and
`KaliumSecretion` biology modules. These are the modules that will govern the
behavior of the simulation objects (i.e. cells). We import them at the top of the
source code with `#include diffusion_biology_modules`.

#### 3. Set up the simulation objects

Next up is creating simulation objects:

``` C++
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetMass(1.0);
    cell.AddBiologyModule(Chemotaxis());
    std::array<double, 3> secretion_position = {{50, 50, 50}};
    if (position == secretion_position) {
      cell.AddBiologyModule(KaliumSecretion());
    }
    return cell;
  };
  std::vector<std::array<double, 3>> positions;
  positions.push_back({0, 0, 0});
  positions.push_back({100, 0, 0});
  positions.push_back({0, 100, 0});
  positions.push_back({0, 0, 100});
  positions.push_back({0, 100, 100});
  positions.push_back({100, 0, 100});
  positions.push_back({100, 100, 0});
  positions.push_back({100, 100, 100});
  // the cell responsible for secretion
  positions.push_back({50, 50, 50});
  ModelInitializer::CreateCells(positions, construct);
```

The `construct` lambda defines the properties of each cell that we create. These can be
physical properties (diameter, mass), but also biological properties and behaviors
(chemotaxis, substance secretion)

In this example, each cell is assigned the `Chemotaxis` behavior. In `diffusion_biology_behaviors.h` you can
check the source code of this module. Basically it makes cells move according to the gradient,
caused by a concentration difference of the substance. One of the cells
(the cell at position `{50, 50, 50}`) will be the one secreting the substance;
it therefore gets assigned the `SubstanceSecretion` behavior.

Furthermore, we define the initial positions of the cells. In this example it is
done explicitly, but one could also generate a grid of cells, or a random distribution
of cells.

### Configure the simulation

Create a `bdm.toml` file in the `diffusion` directory, and copy the following lines
into it:

```
[visualization]
export = true
export_interval = 10

	[[visualize_sim_object]]
	name = "Cell"
	additional_data_members = [ "diameter_" ]

	[[visualize_diffusion]]
	name = "Kalium"
	gradient = true

```

This will enable exporting visualization files, so that we can visualize the
simulation after it has finished. Furthermore, we enable the output of the diameter
of our simulation objects (by default named "Cell"), and the gradient data of the
extracellular diffusion

### Build and run the simulation

!!! note "Note (for Mac OS users)"
    In order for the CLI to be available in your shell environment
    it is necessary to run the following command:

    ```
    source biodynamo.env
    ```

Run the following commands to build and run the simulation.

``` sh
biodynamo run
```

### Visualize the simulation

Load the generated ParaView state file as described in [Section Visualization](visualization.md#export-visualization-files).

From "View", select "Animation Panel". This will display some animation settings
at the bottom of the screen. From the "Mode" select "Real Time".
Then click the Play button at the top of the screen to run the simulation visualization.

<video width="100%" controls>
  <source src="https://cernbox.cern.ch/index.php/s/rzl2Kb4uxny4ZXF/download?path=%2F&files=exported_visualization.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>
