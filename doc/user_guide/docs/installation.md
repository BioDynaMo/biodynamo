# Getting Started

There are two types of installations of BioDynaMo: the **user installation** and
the **developers installation**. Currently supported platforms are Linux and
Mac OS X. Below you will find instructions on how to install each installation
type for the supported platforms.

## User Installation

The user installation is intended for those that want to make use of the
existing features of BioDynaMo and only focus on implementing biological models
with those features.

!!! note
	The installation will also include the visualization software (ParaView). Therefore it is not
	necessary to install ParaView yourself.

### Linux

``` sh
wget https://github.com/BioDynaMo/biodynamo/releases/download/v0.1.0/biodynamo_0.1.0_amd64.snap
sudo snap install --dangerous --classic biodynamo_0.1.0_amd64.snap
```

### Mac OS

``` sh
brew install Biodynamo/biodynamo/biodynamo
```


## Developer Installation

The development installation is intended for those that want to make changes to
BioDynaMo itself, by creating new features or extending existing ones.

### Linux

``` sh
git clone https://github.com/BioDynaMo/biodynamo
cd biodynamo

sudo script/install_prerequesites_ubuntu_16.04.sh

# source biodynamo environment
source /opt/biodynamo/biodynamo_dev.env

mkdir build && cd build
cmake .. && make -j4
sudo make install
```

### Mac OS

!!! note
	The developement installation requires you to have homebrew installed

``` sh
git clone https://github.com/BioDynaMo/biodynamo
cd biodynamo

script/install_prerequesites_macos.sh

# source biodynamo environment
source /opt/biodynamo/biodynamo_dev.env

mkdir build && cd build
cmake .. && make -j4
sudo make install
```
