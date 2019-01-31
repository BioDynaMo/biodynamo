# Parameter

Parameters are used to tailor BioDynaMo to your specific simulation.
Parameters can be categorized into compile time and runtime parameters.

## Compile Time Parameter

Compile time parameters are required to tell BioDynaMo which object types you
want to use in your simulation. BioDynaMo provides a set of macros to reduce the
amount of boilerplate code you have to write.

Compile time parameters can be defined for the whole simulation or for a specific
simulation object. Below, you can find a short example:

``` c++
FIXME
BDM_CTPARAM(experimental::neuroscience) {
  BDM_CTPARAM_HEADER(experimental::neuroscience);

  // overwrite BiologyModules for bdm::Cell
  BDM_CTPARAM_FOR(bdm, Cell) {
    using BiologyModules = CTList<Chemotaxis, SubstanceSecretion>;
  };
};
```

`BDM_CTPARAM` and `BDM_CTPARAM_HEADER` take a list of BioDynaMo modules as
argument. Leave it empty if you only want to use functionality from
BioDynaMo core.

!!! Info
    If you change any of the compile time parameter you have to recompile your
    simulation.


## Runtime Parameter

In contrast to compile time parameter, it is possible to change runtime
parameters without recompilation.

The majority of runtime parameters are defined in the [core engine](https://biodynamo.github.io/api/structbdm_1_1Param.html).
Each module can define its own [`Param` class](https://biodynamo.github.io/api/structbdm_1_1experimental_1_1neuroscience_1_1Param.html) to add additional parameters.

There are three ways to set the value of a runtime parameter:

1.  TOML configuration file
2.  Command line parameter
3.  Assignment in the source code.

!!! Note
    Higher index takes precedence.  E.g. If you define the `backup_file` in the TOML file and the
    command line parameter, the command line version will be used.

The documentation of each parameter contains a description of the
parameter, its default value and how to set it in the TOML file ([example](https://biodynamo.github.io/api/structbdm_1_1Param.html#a3cc70d57ed2965f5551e03b36a4a7219))

The following code snippet shows how to access a runtime parameter in your
simulation.

```
const auto* param = Simulation::GetActive()->GetParam();
std::cout << param->simulation_time_step_ << std::endl;
```


### Configuration File

This is the recommended way to set runtime variables. Create a file `bdm.toml`
in the working directory and add your configuration. You can find a sample below:

```
[visualization]
export = true
export_interval = 1

[[visualize_sim_object]]
name = "Cell"
additional_data_members = [ "density_" ]
```

### Command Line Parameter

Some parameter can be set as command line argument when you start the simulation.
For a complete list execute the binary with the `--help` switch. e.g. `./cell_division --help`.

Sample output:
```
Info: Initialize new simulation using BioDynaMo v0.1.0-105-g74f6a24

-v, --verbose
    Verbose mode. Causes BioDynaMo to print debugging messages.
    Multiple -v options increases the verbosity. The maximum is 3.

-r, --restore filename
    Restores the simulation from the checkpoint found in filename and
    continues simulation from that point.

-b, --backup filename
    Periodically create full simulation backup to the specified file
    NOTA BENE: File will be overriden if it exists

--help
    Print usage and exit.

```

### Assignment in the Source Code

You can also set a runtime parameter in the source code. You have to recompile
your simulation though.

```
auto set_param = [](auto* param) {
  // Create an artificial bounds for the simulation space
  param->bound_space_ = true;
  param->min_bound_ = 0;
  param->max_bound_ = 250;
  param->run_mechanical_interactions_ = false;
};

Simulation simulation(argc, argv, set_param);
```

You have to create a lambda and pass it to the constructor of `Simulation`.
Afterwards, `Simulation` only returns a const pointer to the parameters. Thus,
they cannot be modified.
