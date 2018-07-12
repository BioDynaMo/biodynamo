The most basic BioDynaMo simulation (i.e. our Hello World program) is a
static single cell. It is the default model that comes with the installation of
BioDynaMo. In this section we shall go over the three simple steps of running
this simple simulation.


### Step 1: Create your simulation

Run the following command to create a new project called "hello_world":

``` sh
biodynamo new hello_world
```

### Step 2: Build your simulation

Go into the newly created directory `hello_world` with:

``` sh
cd hello_world
```

And build the project files with:

``` sh
biodynamo build
```

### Step 3: Run your simulation

``` sh
biodynamo run
```

You should see "Simulation completed succesfully" as the output.


### Extra: Cell division

Let's make the simulation more interesting by adding a biological behavior to the
cell: cell division. Open the `src/hello_world.h`

Replace the `CompileTimeParameters` struct with the following one:

``` C++
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<GrowDivide>;
};
```

And add the following line to the `construct` function:

``` C++
cell.AddBiologyModule(GrowDivide(32, 3000, {gAllBmEvents}));
```

Rebuild and rerun the simulation to have the cell division take effect. visit the
next exercise to learn about the visualization features of BioDynaMo.

!!! info
	You might want to run for a few more simulation steps to witness cells dividing
