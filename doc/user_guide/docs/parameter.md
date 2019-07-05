---
title: "Parameter"
date: "2019-01-01"
path: "/biodynamo/doc/user_guide/docs/parameter/"
meta_title: "param"
meta_description: "This is the parameter page."
toc: true
image: ""
next:
    url:  "/biodynamo/doc/user_guide/docs/parameter/"
    title: "Parameter"
    description: "This is the parameter page."
sidebar: "userguide"
keywords:
  -parameter
  -configuration
  -code
  -setup
---

Parameters are used to tailor BioDynaMo to your specific simulation.

The majority of parameters are defined in the [core engine](https://biodynamo.github.io/api/structbdm_1_1Param.html).
Each module can define its own [`Param` class](https://biodynamo.github.io/api/structbdm_1_1experimental_1_1neuroscience_1_1Param.html) to add additional parameters.

There are three ways to set the value of a parameter:

1.  TOML configuration file
2.  Command line argument
3.  Assignment in the source code.

<br>
<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
      <h4><b>Note<b><h4>
      <p>Higher index takes precedence.  E.g. If you define the <code>backup_file</code> in the TOML file and the
    command line parameter, the command line version will be used.
    </p>
    </div>
</a>

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

### Command Line Options

Some parameter can be set as command line argument when you start the simulation.
For a complete list execute the binary with the `--help` switch. e.g. `./cell_division --help`.

Sample output:
```
 -- BioDynaMo command line options

Usage:
  ./cell_division [OPTION...]

 Simulation options:
  -n, --num-cells arg  The total number of cells (default: 10)

 Core options:
  -h, --help          Print this help message.
      --version       Print version number of BioDynaMo.
      --opencl        Enable GPU acceleration through OpenCL.
      --cuda          Enable GPU acceleration through CUDA.
  -v, --verbose       Verbose mode. Causes BioDynaMo to print debugging
                      messages. Multiple -v options increases the verbosity. The
                      maximum is 3.
  -r, --restore FILE  Restores the simulation from the checkpoint found in
                      FILE and continues simulation from that point. (default: )
  -b, --backup FILE   Periodically create full simulation backup to the
                      specified file. NOTA BENE: File will be overriden if it
                      exists. (default: )
  -c, --config FILE   The TOML configuration that should be used. (default: )

```

You can append your own command line options as following (e.g. `num-cells` as
show in the sample above):

```
auto opts = CommandLineOptions(argc, argv);
opts.AddOption<uint64_t>("n, num-cells", "The total number of cells", "10");

// You will need to pass this object to the main Simulation object as follows:
Simulation simulation(opts);

// To retrieve the values from the command line in your simulation code
auto parser = opts.Parse();
uint64_t num_cells = parser.Get<uint64_t>("num-cells");
```

The `AddOption` function can be broken down as follows: The template parameter
(`uint64_t`) specifies what value type your option should be. The first fucntion
parameter is for the flag abbreviation (in this case -n and --num-cells) The
second parameter is the name of your command line option The third parameter is
the description (as shown in the help dialogue) The fourth parameter is the
default value of the command line option (i.e. when you do not specify the
flag).

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
