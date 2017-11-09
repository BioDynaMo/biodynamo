# Usage

There is a slight difference in usage of BioDynaMo between the user-installed
version and the developer-installed version. Below you will find the usage
instructions for the user-installed version. At the [bottom of the page](#notes-for-developers) you can
find some remarks for the developer-installed version.

## Setting up the Command Line Interface

Setting up a simulation and running it is done with the BioDynaMo command line
interface (CLI). 

!!! info "Note (for Mac OS users)"
	In order for the CLI to be available in your shell environment
	it is necessary to run the following command:

	```
	source biodynamo.env
	```

## Basic Workflow

A basic BioDynaMo workflow looks as follows:

#### 1. Create a new project

To create a new project run the following command:

``` sh
biodynamo new <enter_name>
```

This command creates a new Git repository for you with the name specified
in the `<enter_name>` placeholder. It will ask you to login with your Github
credentials to make your project files remotely accessible. You will see a folder appear with the same
name, containing some template files to get you started.

!!! tip
	If you wish not to have your Github account linked to your project you can
	append the `--no-github` option to the command. You will however not be able
	to use the `biodynamo assist` command as explained below.

#### 2. Implement your model

The provided template files in your newly created folder can be used as a
starting point for your simulation. The `src` directory contains the files with
the source code that defines the simulation. You can edit and add the code that
describes your model in this folder.

To find out about the features that BioDynaMo offers, please check out the [Documentation](documentation.md).
For example projects, see the [Exercises](hello_world.md)

#### 3. Run the simulation

Building the simulation and running it can be done with the command:

``` sh
biodynamo run
```

All your source code will be compiled and linked against the BioDynaMo libraries, and an executable file
will be created and run. It is at this point you might encounter compilation errors.
You will need to fix them before the simulation can actually run.


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


## Notes for developers
If you have installed the developers version and you want to make use of your
custom features then the procedure is similar to the user-installed usage. The
only extra step that needs to be performed is the following one at the beginning:

``` sh
source /opt/biodynamo/biodynamo_dev.env
```

!!! note "Mac OS users"
	You will **not** need to `source biodynamo.env` after this.
