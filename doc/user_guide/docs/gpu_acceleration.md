---
title: "GPU Acceleration"
date: "2019-01-01"
path: "/docs/userguide/gpu_acceleration/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the gpu acceleration page."
toc: true
image: ""
next:
    url:  "/docs/userguide/gpu_acceleration/"
    title: "GPU Acceleration"
    description: "This is the gpu acceleration page page."
sidebar: "userguide"
keywords:
  -gpu
  -acceleration
  -faster
  -speed
  -up
---

## GPU-acceleration: what and why?

Physical interactions are one of the most compute intensive operations in
biological simulations. The main reason is that the computations that are
involved often include operators that require multiple CPU cycles to perform the corresponding operation. In an accurate model, physical interactions happen between all simulation objects and their local environment, for every time step. Therefore, you can imagine that a lot of performance can be gained by accelerating these interactions.

General purpose GPUs (GPGPUs) make it possible to obtain the computing performance of a small cluster computer. Almost any desktop computer, or laptop has a built-in GPU available. It mostly takes care of all the graphical computations that take place on a computer, but recent developments allow us to use GPUs for high-performance computing purposes. Frameworks such as CUDA and OpenCL make it possible to program a GPU to perform the computations that we specify at a speed much higher than on a regular CPU. Of course, this depends on the type of computation that you want to perform, but fortunately physical interactions in BioDynaMo fits the bill.

## Requirements
We try to keep things as simple as possible in terms of usage. So for the most part you are good to go as long as you meet either the following requirements:

- You have a CUDA-compatible GPU and CUDA installed
- You have an OpenCL-compatible GPU and OpenCL installed

If you have multiple GPUs on your machine, BioDynaMo will automatically select one. You can also configure this yourself if you prefer one GPU over the other(s). More on this below.

## Enabling GPU acceleration
The only thing you need to do to enjoy GPU acceleration is enabling it through the configuration file (bdm.toml) as following:

```Python
[experimental]
use_gpu = true
```

By default we assume that your GPU is only CUDA-compatible. If you want to let BioDynaMo know you have an OpenCL-compatible GPU just append the following to the above snippet:

```Python
use_opencl = true
```
<br/>
<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Note</b></h4>
    	<p>If you have multiple GPUs on your system you can select which BioDynaMo uses by setting the following flag: <code>preferred_gpu = &#60;value&#62;</code>, where <code>&#60;value&#62;</code> is the index of the GPU in the list of all GPUs.
		</p>
    </div>
</a>
