# analytic-continuum

This demo demonstrates how to interface a arbitrary continuum model with 
BioDynaMo. For simplicity, we restrict ourself to a one way coupling with an 
analytical continuum models. 
We refer to the coupling as one-way because the agents react to the continuum 
but the continuum does not react to the agents.
Numerical schemes can be integrated in a similar 
fashion, see for instance the finite difference scheme implemented in 
`DiffusionGrid` or `EulerGrid`. 

The demo shows roughly 100000 Agents that register the values of the continuum
and save the value as a member such that we can visualize it. The continuum 
model is 
```
f(x,y,z,t) = (1-e^{-t}) sin(w_x*x) * sin(w_y*y) .
```
Here, the agents only remember the value, but it's clear that one can easily 
construct more complex behaviors based on the continuum information.

## 1. Sourcing BioDynaMo

Whenever you interact with this repository, make sure you have sourced BioDynaMo
correctly. Anytime that you open a new terminal, you have to source it again. 
```bash
. <path_to_biodynamo>/build/bin/thisbdm.sh
```

You can verify that BioDynaMo has been sourced correctly by running 
`bdm -v`. 

## 1. Building the simulation and the tests

```bash
bdm build
```

## 2. Building and running the simulation

```bash
bdm run
```
Note that this automatically calls `bdm build`.

## 3. Visualize the results

Open ParaView in you project directory `<path>/analytic-continuum` with
```bash
paraview
```
Then
```
File > Load State > "output/analytic-continuum/analytic-continuum.pvsm > OK > OK
```
You should now see many grey agents. On the left hand side, you should see your
`Pipeline Browser`. Select `ContinuumRetrieverAgents`. Below, you find a 
`Properties` window. Navigate to `Coloring` and change `Solid Color` to 
`my_continuum_value_`. Then, jump to the last time step by clicking the 
right most green arrow in top row of ParaView. We rescale the color map
to the values in the simulation by clicking `Rescale to Data Range`, again, in 
the `Coloring` tap (third button). We can now jump back to the first time step 
and click `Play` to visualize the entire simulation over time.
