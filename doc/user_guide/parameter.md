---
title: "Parameter"
date: "2019-01-01"
path: "/docs/userguide/parameter/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the parameter page."
toc: true
image: ""
sidebar: "userguide"
keywords:
  -parameter
  -configuration
  -code
  -setup
---

Parameters are used to tailor BioDynaMo to your specific simulation.

The majority of parameters are defined in the [core engine](/api/structbdm_1_1Param.html).
Each simulation or module can define its own [`Param` class](/api/structbdm_1_1experimental_1_1neuroscience_1_1Param.html) to add additional parameters.

There are three ways to set the value of a parameter:

1.  TOML/JSON configuration file
2.  Command line argument
3.  Assignment in the source code.

<br/>
<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
      <h4><b>Note</b></h4>
      <p>Higher index takes precedence.  E.g. If you define the <code>backup_file</code> in the TOML/JSON file and the
    command line parameter, the command line version will be used.
    </p>
    </div>
</a>

The documentation of each parameter contains a description of the
parameter, its default value and how to set it in the TOML file ([example](/api/structbdm_1_1Param.html#a13d24f045335b7ac62a091f56c6fe166))

The following code snippet shows how to access a parameter in your
simulation.

```cpp
const auto* param = Simulation::GetActive()->GetParam();
std::cout << param->simulation_time_step << std::endl;
std::cout << param->GetModuleParam<neuroscience::Param>()->neurite_max_length_ << std::endl;

```


### Configuration File

This is the recommended way to set runtime variables. 
BioDynaMo supports configuration files in [TOML](https://toml.io/en/) 
or [JSON merge patch](https://tools.ietf.org/html/rfc7386) format.

Here you can find a tutorial about the usage of [simulation parameters](/docs/userguide/simulation_parameter_tutorial)

#### TOML

Create a file `bdm.toml`
in the working directory and add your configuration. You can find a sample below:

```toml
[visualization]
export = true

[[visualize_agent]]
name = "Cell"
additional_data_members = [ "density_" ]
```

#### JSON

Create a file `bdm.json`
in the working directory and add your configuration. You can find a sample below:

```json
{
  "bdm::Param": {
    "export_visualization": true,
    "visualize_agents": {
      "Cell": ["density_"]
    }
  }
}
```

### Command Line Options

Some parameter can be set as command line argument when you start the simulation.
For a complete list execute the binary with the `--help` switch. e.g. `./cell_division --help`.

Sample output:
```
-- BioDynaMo command line options

Usage:
  ./cell_division [OPTION...]

 Core options:
  -h, --help                    Print this help message.
      --version                 Print version number of BioDynaMo.
      --opencl                  Enable GPU acceleration through OpenCL.
      --cuda                    Enable GPU acceleration through CUDA.
  -v, --verbose                 Verbose mode. Causes BioDynaMo to print
                                debugging messages. Multiple -v options increases
                                the verbosity. The maximum is 3.
  -r, --restore FILE            Restores the simulation from the checkpoint
                                found in FILE and continues simulation from
                                that point. (default: )
  -b, --backup FILE             Periodically create full simulation backup to
                                the specified file. NOTA BENE: File will be
                                overriden if it exists. (default: )
  -c, --config FILE             The TOML or JSON configuration that should be
                                used. The JSON file must be in JSON merge
                                patch format
                                (https://tools.ietf.org/html/rfc7386) (default: )
      --inline-config JSON_STRING
                                JSON configuration string passed directly on
                                the command line. Overwrites values specified
                                in config file.  The JSON string must be in
                                JSON merge patch format
                                (https://tools.ietf.org/html/rfc7386) (default: )
      --output-default-json     Prints a JSON string with all parameters and
                                their default values and exits.
      --toml-to-json TOML_FILE  Converts a TOML file to a JSON patch. After
                                printing the JSON patch the application will
                                exit. (default: )
```

You can append your own command line options as following (e.g. `num-cells` as
show in the sample above):

```cpp
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

```cpp
auto set_param = [](Param* param) {
  // Create an artificial bound for the simulation space
  param->bound_space = true;
  param->min_bound = 0;
  param->max_bound = 250;
  param->run_mechanical_interactions = false;
};

Simulation simulation(argc, argv, set_param);
```

You have to create a lambda and pass it to the constructor of `Simulation`.
Afterwards, `Simulation` only returns a const pointer to the parameters. Thus,
they cannot be modified.
