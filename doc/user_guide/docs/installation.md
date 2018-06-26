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

## Supported platforms

*  Ubuntu 16.04, 18.04
*  Mac OSX
