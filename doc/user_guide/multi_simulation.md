---
title: "BioDynaMo Multi Simulation"
date: "2021-11-18"
path: "/docs/userguide/multi_simulation/"
meta_title: "BioDynaMo User Guide"
meta_description: "Information about BioDynaMo Multi Simulation runtime"
toc: true
image: ""
sidebar: "userguide"
keywords:
  -multi
  -simulation
  -multi-simulation
---

# Multi Simulation: What and Why?

With BioDynaMo it is possible to run multiple simulations in parallel, as
separate processes. We refer to this as "Multi Simulation". This can be a
useful feature when you want to repeat a certain simulation multiple times;
possibly with different runtime parameters. Instead of running the simulations
consecutively, with Multi Simulation it is possible to run multiple instances simultaneously, on one or more machines. This allows you to explore a parameter
space and obtain results at a much faster pace. The more computing resources are
available, the faster the simulations are completed.
BioDynaMo offers a couple of default algorithms on exploring a parameter space (e.g. parameter sweep, particle swarm).
The algorithms benefit from Multi Simulation by running each iteration in a separate process.

## MPI
Multi Simulation uses MPI to spawn different processes and schedule the
simulations on your system(s).
    
    If you work with more than one machine, you are required to have the
    same OpenMPI versions installed on all the machines in order for Multi Simulation to work properly.

    Furthermore, MPI requires you to have a passwordless SSH login to the other machines.

## How to use Multi Simulation

In order to run a simulation in Multi Simulation mode, you need to make a few changes.

### Main function
In the `main` function of your simulation you need to call the `MultiSimulation` wrapper
around your regular `Simulate` call:

```c++
int main(int argc, const char** argv) {
  bdm::experimental::MultiSimulation pe(argc, argv);
  return pe.Execute(Simulate);
}
```

### Simulate function

Your `Simulate` function also should conform to the following signature in order to use Multi Simulation:

```c++
void Simulate(int argc, const char** argv, TimeSeries* result,
                     Param* final_params = nullptr) {
auto set_param = [&](Param* param) {
    param->Restore(std::move(*final_params));
};
Simulation simulation(argc, argv, set_param);

// Your simulation code...
}
```

`result ` is a `TimeSeries` object that can be populated with results that are of interest in your simulation.
For some optimization algorithms, like ParticleSwarm, this *must* be populated with such results to be able to minimize the error between simulated data and real-life data.

`final_params` are the unique set of parameters that a simulation instance receives from the Multi Simulation runtime. You must therefore configure your simulation with these parameters *before* you define your simulation using the `Restore` functionality.

### Optimization parameters

The Multi Simulation runtime expects you to define which parameter space exploration algorithm you want to use.
This can be done by defining a `OptimizationParam` in your parameter configuration:

```json
{
  "bdm::OptimizationParam": {
    "algorithm" : "<algorithm name>",
    "params" : [
      {
        "_typename": "<parameter type>",
        "param_name" : "<parameter name>",
        ...
      }
    ]
  }
}

```
* The `algorithm` should be the name of the algorithm you wish to use (e.g. "ParameterSweep", "ParticleSwarm")
* The `params` should be a list of simulation parameters you wish to explore. In the code block above, there is only one parameter block, but this can be a comma-separated list of multiple parameter blocks.
  * The `_typename` should be the type of parameter it concerns (e.g. a uniform range of parameters, a set of parameters). See "core/multi\_simulation/optimization\_param_type" for a list of available parameter types
  * The `param_name` should be the name of the parameter in your simulation, including any namespace identifiers (e.g. "bdm::SimParam::my_param")
  * Each parameter type has a number of extra fields that need to be filled in. For example, for a RangeParam (a uniform range of values), you would also need to specify the `lower_bound`, `upper_bound` and `stride` in its parameter block.

### Command line execution
Since Multi Simulation relies on MPI, we must use the `mpirun` binary to run the simulation with

``` sh
$ mpirun -np <N> <your_simulation_binary> --config=param.json
```

`param.json` contains the optimization parameter information as described earlier (this can also instead be inlined with the `inline-config` command line argument).

This command spawns `N-1` number of processes that each will start a
simulation (`N-1` because one process is always reserved as the managing process). The number of simulations that will eventually be performed depends
on your optimization algorithm: the more parameters you would like to explore,
the higher the number of simulations that need to be executed.

It is very likely that the total number of simulations will be greater than `N-1`. Since
there can only be at most `N-1` simulation running at any given point in time,
the other simulations are queued, and scheduled to be executed whenever
computing resources become available.

If you wish to run your simulations on multiple machines (e.g. a cluster, or cloud instances), it
can simply be done as follows:

``` sh
$ mpirun -np <N> --hostfile <path/to/hostfile> <your_simulation_binary> --config=param.json
```

The `hostfile` should contain the names (e.g. IP addresses) of the machines in
your cluster. For detailed information on how to create a hostfile, please check
out the OpenMPI docs [^1].

  [^1]: https://www.open-mpi.org/faq/?category=running#mpirun-hostfile

### Performance tuning

Since we rely on MPI to distribute the simulation workloads over a specified
number of processes, we are able to finetune the performance through the many
options that come with the MPI implementation. Important for the performance of
multi-threaded applications is being able to tweak the number of threads that
each process is able to spawn, the thread affinity, and other related
parameters. These parameters are conveniently described online [^3].

  [^3]: https://www.open-mpi.org/doc/v3.0/man1/mpirun.1.php



