# Makefile projects

This demo shows how to compile a BioDynaMo simulation using a Makefile project
instead of our standard CMake. Thus, BioDynaMo can be integrated   
into software packages that use different build systems.
However, if you can choose freely we highly recommend to use CMake for the best
experience.

Start this demo with [installing BioDynaMo](https://biodynamo.github.io/biodynamo/installation/).
To compile the simulation change into this directory and execute `make`.
After a few seconds the binary `my-simulation` should have been created.
You can run it by executing `./my-simulation`.
If everything was successful, the final output should be:
Simulation completed successfully!


The solution uses two helper scripts `bdm-config` and `bdm-code-generation`.
`bdm-config` provides the compiler and linker options that are required to build
the simulation.
`bdm-code-generation` is performing the code generation step that is usually
taken care of by CMake. BioDynaMo relies on code generation to support reflection.
