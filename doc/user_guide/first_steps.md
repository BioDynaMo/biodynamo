---
title: "First Steps"
date: "2019-01-01"
path: "/docs/userguide/first_steps/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the first steps page."
toc: true
image: ""
next:
    url:  "/docs/userguide/first_steps/"
    title: "First Steps"
    description: "This is the first steps page."
sidebar: "userguide"
keywords:
  -tutorial
---

Setting up a simulation and running it is done with the BioDynaMo command line
interface (CLI). Open a new terminal and execute:

```bash
source <installation-directory>/bin/thisbdm.sh
```

This command must be executed whenever you want to use BioDynaMo in a new terminal!
By default `<installation-directory>` will be `$HOME/biodynamo-vX.Y.Z`, where X.Y.Z is the version number


## Basic Workflow

A basic BioDynaMo workflow looks as follows:

### 1. Create a new project

To create a new project run the following command:

```bash
biodynamo new <enter_name>
```

This command creates a new Git repository for you with the name specified
in the `<enter_name>` placeholder. You will see a folder appear with the same
name, containing some template files to get you started.

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Tip</b></h4>
    	<p>If you wish to have your Github account linked to your project you can
	append the <code>--github</code> option to the command. 
		</p>
    </div>
</a>

### 2. Implement your model

The provided template files in your newly created folder can be used as a
starting point for your simulation. The `src` directory contains the files with
the source code that defines the simulation. You can edit and add the code that
describes your model in this folder.

### 3. Run the simulation

Building the simulation and running it can be done with the command:

```bash
biodynamo run
```

All your source code will be compiled and linked against the BioDynaMo libraries, and an executable file
will be created and run. It is at this point you might encounter compilation errors.
You will need to fix them before the simulation can actually run.

## Try out some demos

There are some demos in the installation. They can be listed with the command:

```bash
biodynamo demo
```

Each of these demos can be copied out to a directory and executed with two `biodynamo` commands:

```bash
biodynamo demo <name> [target]
cd <destination>  # as printed out by the previous command
biodynamo run
```

For example, to run the demo `cell_division`, we can do:

```bash
biodynamo demo cell_division /tmp
cd /tmp/cell_division
biodynamo run
```

