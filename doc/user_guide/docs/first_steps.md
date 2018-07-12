# First Steps

Setting up a simulation and running it is done with the BioDynaMo command line
interface (CLI). Open a new terminal and execute:

``` sh
source <path-to-bdm-installation>/biodynamo-env.sh
```

This command must be executed whenever you want to use BioDynaMo in a new
terminal!


## Basic Workflow

A basic BioDynaMo workflow looks as follows:

#### 1. Create a new project

To create a new project run the following command:

``` sh
biodynamo new <enter_name>
```

This command creates a new Git repository for you with the name specified
in the `<enter_name>` placeholder. You will see a folder appear with the same
name, containing some template files to get you started.

!!! tip
	If you wish to have your Github account linked to your project you can
	append the `--github` option to the command. `biodynamo assist` command requires `--github`

#### 2. Implement your model

The provided template files in your newly created folder can be used as a
starting point for your simulation. The `src` directory contains the files with
the source code that defines the simulation. You can edit and add the code that
describes your model in this folder.

#### 3. Run the simulation

Building the simulation and running it can be done with the command:

``` sh
biodynamo run
```

All your source code will be compiled and linked against the BioDynaMo libraries, and an executable file
will be created and run. It is at this point you might encounter compilation errors.
You will need to fix them before the simulation can actually run.

## Try out some demos

There are some demos in the installation. They can be listed with the command:

```sh
biodynamo demo
```

Each of these demos can be copied out to a directory and executed with two `biodynamo` commands:

```sh
biodynamo demo <name> [target]
cd <destination>  # as printed out by the previous command
biodynamo run
```

For example, to run the demo `cell_division`, we can do:

```sh
biodynamo demo cell_division /tmp
cd /tmp/cell_division
biodynamo run
```

## Request assistance

It can happen that you encounter an error or issue that you cannot solve on your own.
Or you might be convinced that there is a bug in BioDynaMo.
For such occasions we have provided you with BioDynaMo assistance:

``` sh
biodynamo assist
```

This command will create a folder called `debug` in which debug information will be collected.
A git branch will be created and uploaded to Github. Send the link to this branch to us,
so we can take a look at the issue. Check out [Contact](contact.md) on how to contact us.
