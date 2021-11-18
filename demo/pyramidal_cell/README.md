# Pyramidal Cell Demo

This demo simulates the growth of a pyramidal cell. 

To compile and run the simulation, execute the following command in the terminal.

```
bdm run
```

The simulation will use the `bdm.json` parameter file and will create
visualization files in directory `output/pyramidal_cell`.

To render an image of the final neuron, execute:

```
pvbatch ./pv_script.py --screenshots
```

The `pv_script.py` loads the visualization files and adjusts the default settings (e.g. camera, background color, iteration step, and more).
The additional parameter `--screenshots`, tells the script to render an image.
`pvbatch` executes this script without opening a window.

To interactively explore the simulation output with the default visualization settings, execute: 

```
paraview output/pyramidal_cell/pyramidal_cell.pvsm
```

To use the same settings as in the `pvbatch` step, execute:

```
paraview --script=./pv_script.py
```

For more information about the simulation itself have a look at the following publication:

Lukas Breitwieser et al. BioDynaMo: a modular platform for high-performance agent-based simulation.
Bioinformatics, 2021. DOI: https://doi.org/10.1093/bioinformatics/btab649.
