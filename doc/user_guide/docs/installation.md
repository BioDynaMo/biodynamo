# Installation

To install BioDynaMo for the first time execute the following commands.
The installation will also install all required packages including Paraview.

``` sh
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
./install.sh
```

!!! important

    1. After the installation you need to restart your terminal.
      In every new terminal execute `source <path-to-bdm-installation>/biodynamo-env.sh`
      to use BioDynaMo!

    2. BioDynaMo uses a customized version of ParaView.
	     Therefore, you should not install ParaView separately.

## Update Installation

The following commands update your BioDynaMo installation:

``` sh
cd path/to/biodynamo
# make sure you are on the master branch
git checkout master
# get latest changes
git pull origin master
./install.sh
```
## Checking your files compared to the master branch

Once you are already on the master branch you can also check which files of yours are different to those present within the master branch. This can be done by executing the following in the terminal :

```
git status

```
This will list all files you have that are not present within the master branch, it is always a good idea to check this before doing any form of updating so that you can back up your own modules e.t.c

## Supported platforms

*  **Ubuntu 16.04 (recommended)**, 18.04
*  Mac OSX

### Experimental

*  CentOS 7.5.1804 (no ParaView support at the moment)
