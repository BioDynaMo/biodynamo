---
title: "Simulation Parameter Tutorial"
date: "2020-09-08"
path: "/docs/userguide/simulation_parameter_tutorial/"
meta_title: "BioDynaMo User Guide"
meta_description: "This page explains simulation parameters."
toc: true
image: ""
sidebar: "userguide"
keywords:
  -parameter
  -json
---

This tutorial demonstrates the usage of simulation parameters using the JSON format.

### Copy the demo code

`parameters` is one of many installed demos in BioDynaMo. It can be copied out
with `biodynamo demo`.

```bash
biodynamo demo parameters
```

### Inspect the code

Go into the `parameters` directory and open the files `src/parameters.h` 
and `src/parameters.cc` in your 
favorite editor and inspect the code.
We can note the following things from its content:

#### 1. Add simulation specific parameters

We add the simulation specific parameters `foo` and `bar`.
Therefore, we create a new struct called `SimParam`, which inherits
from `ParamGroup`.
To reduce the amount of boilerplate code, BioDynaMo provides the macro
`BDM_PARAM_GROUP_HEADER`.

```cpp
// Parameters specific for this simulation
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  double foo = 3.14;
  int bar = -42;
};
```

#### 2. Initialize ParamGroupUid

Every ParamGroup must have a unique identifier.
Definition of `SimParam::kUid` must be done in a source file (`src/parameters.cc`).

```cpp
const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();
```

#### 3. Register new parameters

Before we create a simulation, we have to tell BioDynaMo about the new parameters.

```cpp
Param::RegisterParamGroup(new SimParam());

```

#### 4. Create simulation

We create a new simulation and pass the command line arguments to it.

```cpp
Simulation simulation(argc, argv);

```

#### 5. Print parameters

We obtain pointers to the core and our simulation specific parameters
and print out a few values.

```cpp
// get a pointer to the param object
auto* param = simulation.GetParam();
// get a pointer to an instance of SimParam
auto* sparam = param->Get<SimParam>();

std::cout << "Value of simulation time step " << param->simulation_time_step << std::endl;
std::cout << "Value of foo                  " << sparam->foo << std::endl;
std::cout << "Value of bar                  " << sparam->bar << std::endl;
```

NB: If you don't have a pointer to `bdm::Simulation` inside e.g. a behavior, you can 
obtain it by calling `Simulation::GetActive()`.

### Build the simulation

In this tutorial, we use `cmake` and `make` directly instead of `biodynamo build` or `biodynamo run`.

```bash
mkdir build
cd build
cmake ..
make -j4
```

These commands create a binary called `parameters`

### Run the simulation

Execute `./parameters` in your build directory.
You should see the following output.

```
Value of simulation time step 0.01
Value of foo                  3.14
Value of bar                  -42
Simulation completed successfully!
```

These three parameters are set to their default values.

### Explore available parameters

BioDynaMo comes with a command line option to print all available simulation parameters.
Execute `./parameters --output-default-json`.

You should see output similar to this:

```json
Below you can find a JSON string with all available parameters and their default values.
Have a look at https://biodynamo.org/bioapi/ for more details about each parameter.
{
    "bdm::Param": {
        "_typename": "bdm::Param",
        "backup_file": "",
        "backup_interval": 1800,
        "bound_space": false,
        "cache_neighbors": false,
        "calculate_gradients": true,
        "compute_target": "cpu",
        "debug_numa": false,
        "detect_static_agents": false,
        "diffusion_type": "Euler",
        "export_visualization": false,
        "leaking_edges": true,
        "max_bound": 100,
        "mem_mgr_aligned_pages_shift": 8,
        "mem_mgr_growth_rate": 1.1,
        "mem_mgr_max_mem_per_thread": 10485760,
        "min_bound": 0,
        "minimize_memory_while_rebalancing": true,
        "numerical_ode_solver": 1,
        "opencl_debug": false,
        "output_dir": "output",
        "preferred_gpu": 0,
        "restore_file": "",
        "root_visualization": false,
        "scheduling_batch_size": 1000,
        "show_simulation_step": true,
        "simulation_max_displacement": 3,
        "simulation_step_freq": 10,
        "simulation_time_step": 0.01,
        "agent_uid_defragmentation_high_watermark": 0.9,
        "agent_uid_defragmentation_low_watermark": 0.5,
        "statistics": false,
        "thread_safety_mechanism": 1,
        "use_bdm_mem_mgr": true,
        "visualization_engine": "paraview",
        "visualization_export_generate_pvsm": true,
        "visualize_diffusion": [],
        "visualize_agents": {
            "_typename": "map<string,set<string> >"
        }
    },
    "bdm::SimParam": {
        "_typename": "bdm::SimParam",
        "bar": -42,
        "foo": 3.14
    }
}

```

NB: You can ignore keys with name `_typename`

### Create a JSON config file

Now let's create a JSON config file. The output from the previous step can serve as a good
starting point. 

Create a new file `bdm.json` in the project root directory with the following content.

```json
{
  "bdm::Param": {
     "simulation_time_step": 1.0
  },
  "bdm::SimParam": {
    "bar": 84
  }
}
```

Execute `./parameters` in your build directory.
You should see the following output.

```
Value of simulation time step 1
Value of foo                  3.14
Value of bar                  84
Simulation completed successfully!
```

BioDynaMo automatically picked up the configutation file and parsed its content.
BioDynaMo looks for a file with the name `bdm.json` in the current working directory and 
it's parent directory. If it finds one it automatically uses it.

### Use specific JSON config file

Rename the file `bdm.json` from the previous step to `config.json`.

If you execute `./parameters` again, you get the same output as running [without config file](#run-the-simulation).

BioDynaMo has different methods to choose a specific configuration file.
This allows us to have multiple configuration files in the same project.

Execute `./parameters --config ../config.json` from within the build directory.
You should see the following output.

```
Value of simulation time step 1 
Value of foo                  3.14
Value of bar                  84
Simulation completed successfully!
```

There is another way to tell BioDynaMo to use a specific configuration file. You can pass a config file parameter as an argument to the `bdm::Simulation` constructor.
 
Therefore, in file `src/parameters.h` replace `Simulation simulation(argc, argv)`
with `Simulation simulation(argc, argv, {"../config.json"})`.
Since we changed the code, we have to recompile it before we can execute it.

```
make -j4
./parameters
```

Although we didn't specify any command line argument, BioDynaMo still picks up our `config.json` configuration file.

```
Value of simulation time step 1 
Value of foo                  3.14
Value of bar                  84
Simulation completed successfully!
```

### Use inline config command line parameter

In addition to a configuration file, BioDynaMo offers the functionality to specify the JSON string on the command line using the
`inline-config` option.


Execute the following command to change the `foo` parameter on the command line.
```bash
./parameters --inline-config '{ "bdm::SimParam": { "foo": 6.28 } }'
```

You should see the following output.

```
Value of simulation time step 1 
Value of foo                  6.28
Value of bar                  84
Simulation completed successfully!
```

We can see that parameter `foo` was correctly updated to `6.28`. The other parameters are set to the values in `config.json`.
If the same value is set in the configuration file and the `inline-config` parameter, the `inline-config` value takes precedence.

### Output used simulation parameters 

BioDynaMo offers the functionality to output simulation metadata at the end of the simulation.
This contains a list of all parameters with their values.

To turn it on update the file `config.json`:

```json
{
  "bdm::Param": {
     "statistics": true,
     "simulation_time_step": 1.0
  },
  "bdm::SimParam": {
    "bar": 84
  }
}
```

Once more, execute `./parameters --inline-config '{ "bdm::SimParam": { "foo": 6.28 } }'`.
You should see output similar to this:

```json
Value of simulation time step 1
Value of foo                  6.28
Value of bar                  84
Simulation completed successfully!

***********************************************
***********************************************
Simulation Metadata:
***********************************************

General
Command                       : ./parameters --inline-config { "bdm::SimParam": { "foo": 6.28 } } 
Simulation name               : parameters
Number of iterations executed : 0
Number of agents  : 0
Output directory              : output/parameters
  size                        : 4.0K

***********************************************

No statistics were gathered!

***********************************************

Thread Info
max_threads             : 4
num_numa nodes          : 1
thread to numa mapping  : 0 0 0 0 
thread id in numa node  : 0 1 2 3 
num threads per numa    : 4 

***********************************************

Agents per numa node
numa node 0 -> size: 0

***********************************************

Parameters
{
    "bdm::Param": {
        "_typename": "bdm::Param",
        "backup_file": "",
        "backup_interval": 1800,
        "bound_space": false,
        "cache_neighbors": false,
        "calculate_gradients": true,
        "compute_target": "cpu",
        "debug_numa": false,
        "detect_static_agents": false,
        "diffusion_type": "Euler",
        "export_visualization": false,
        "leaking_edges": true,
        "max_bound": 100,
        "mem_mgr_aligned_pages_shift": 8,
        "mem_mgr_growth_rate": 1.1,
        "mem_mgr_max_mem_per_thread": 10485760,
        "min_bound": 0,
        "minimize_memory_while_rebalancing": true,
        "numerical_ode_solver": 1,
        "opencl_debug": false,
        "output_dir": "output",
        "preferred_gpu": 0,
        "restore_file": "",
        "root_visualization": false,
        "scheduling_batch_size": 1000,
        "show_simulation_step": true,
        "simulation_max_displacement": 3,
        "simulation_step_freq": 10,
        "simulation_time_step": 1,
        "agent_uid_defragmentation_high_watermark": 0.9,
        "agent_uid_defragmentation_low_watermark": 0.5,
        "statistics": true,
        "thread_safety_mechanism": 1,
        "use_bdm_mem_mgr": true,
        "visualization_engine": "paraview",
        "visualization_export_generate_pvsm": true,
        "visualize_diffusion": [],
        "visualize_agents": {
            "_typename": "map<string,set<string> >"
        }
    },
    "bdm::SimParam": {
        "_typename": "bdm::SimParam",
        "bar": 84,
        "foo": 6.28
    }
}
***********************************************
***********************************************

```

The values in the parameter list are the final values that were used in the simulation.
This includes applying a config file, command line arguments, or hardcoded parameters in the source code.

