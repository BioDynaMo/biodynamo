# Parameter

Parameters are used to tailor BioDynaMo to your specific simulation.

The majority of parameters are defined in the [core engine](https://biodynamo.github.io/api/structbdm_1_1Param.html).
Each module can define its own [`Param` class](https://biodynamo.github.io/api/structbdm_1_1experimental_1_1neuroscience_1_1Param.html) to add additional parameters.

There are three ways to set the value of a parameter:

1.  TOML configuration file
2.  Command line parameter
3.  Assignment in the source code.

!!! Note
    Higher index takes precedence.  E.g. If you define the `backup_file` in the TOML file and the
    command line parameter, the command line version will be used.

The documentation of each parameter contains a description of the
parameter, its default value and how to set it in the TOML file ([example](https://biodynamo.github.io/api/structbdm_1_1Param.html#a3cc70d57ed2965f5551e03b36a4a7219))

The following code snippet shows how to access a parameter in your
simulation.

```
const auto* param = Simulation::GetActive()->GetParam();
std::cout << param->simulation_time_step_ << std::endl;
std::cout << param->GetModuleParam<neuroscience::Param>()->neurite_max_length_ << std::endl;

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
auto set_param = [](Param* param) {
  // Create an artificial bound for the simulation space
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
