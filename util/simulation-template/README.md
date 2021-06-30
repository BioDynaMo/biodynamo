# my-simulation

This repository contains a template for a BioDynaMo simulation. It consists of 
the two folders `src/` and `test/`. The `src/` folder contains the simulation. 
All custom classes and functions that users create to simulate a system 
should ideally end up here. The `test/` folder contains examples for unit tests.
We strongly encourage our users to follow a test driven development process, 
i.e. create unit tests for all fundamental building blocks of your simulation.
By doing so, you can always be sure that a certain function or a class behaves 
as you expect it to. The `.cc` files in `test/` are automatically linked against 
the GoogleTest framework. For more information, please consult the appropriate
[GitHub](https://github.com/google/googletest) page or the 
[Googletest primer](https://google.github.io/googletest/primer.html).

Whenever you interact with this repository, make sure you have sourced BioDynaMo
correctly. If it's sourced, you'll see a `[bdm-1.X.YY]` in your terminal. 
Anytime that you open a new terminal, you have to source it again. 
```bash
. <path_to_biodynamo>/build/bin/thisbdm.sh
```

In the following, we want to explain how to build, run, and test your 
simulation.

## 1. Building the simulation and the tests

Option 1:
```bash
biodynamo build
```

Option 2:
```bash
mkdir build && cd build
cmake ..
make -j <number_of_processes_for_build>
```

## 2. Running the simulation

Before running the simulation, you need to build it. If you haven't done so, 
please go back to step 1.

Option 1:
```bash
biodynamo run
```

Option 2:
```
./build/my-simulation
```

## 3. Execute the unit tests

Before running the unit tests, you need to build them. If you haven't done so, 
please go back to step 1.

Option 1:
```bash
biodynamo test
```

Option 2:
```bash
./build/my-simulation-test
```

Option 2:
```bash
cd build && ctest
```