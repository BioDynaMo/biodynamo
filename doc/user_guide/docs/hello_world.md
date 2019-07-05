---
title: "Hello World"
date: "2019-01-01"
path: "/biodynamo/doc/user_guide/docs/hello_world/"
meta_title: "hello_world"
meta_description: "This is the hello world page."
toc: true
image: ""
next:
    url:  "/biodynamo/doc/user_guide/docs/hello_world/"
    title: "Hello World"
    description: "This is the hello world page."
sidebar: "userguide"
keywords:
  -hello world
  -start
  -easy
  -program
  -first
  -step
---

The most basic BioDynaMo simulation (i.e. our Hello World program) is a
static single cell. It is the default model that comes with the installation of
BioDynaMo. In this section we shall go over the three simple steps of running
this simple simulation.


## Step 1: Create your simulation

Run the following command to create a new project called "hello_world":

```bash
biodynamo new hello_world
```

## Step 2: Build your simulation

Go into the newly created directory `hello_world` with:

```bash
cd hello_world
```

And build the project files with:

```bash
biodynamo build
```

## Step 3: Run your simulation

```bash
biodynamo run
```

You should see "Simulation completed succesfully" as the output.


## Extra: Cell division

Let's make the simulation more interesting by adding a biological behavior to the
cell: cell division. Open the `src/hello_world.h` and add the following line to
the simulate function after the cell is created.

```C++
// Add the biological behavior to the cell.
cell.AddBiologyModule(new GrowDivide(32, 3000, {gAllEventIds}));
```

Rebuild and rerun the simulation to have the cell division take effect. visit the
next exercise to learn about the visualization features of BioDynaMo.

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Info<b><h4>
    	<p>You might want to run for a few more simulation steps to witness cells dividing
		</p>
    </div>
</a>
