# Getting Started

## Installation

Currently supported platforms are Linux Ubuntu and Mac OS X.

### Ubuntu

``` sh
wget https://github.com/BioDynaMo/biodynamo/releases/download/v0.1.0/biodynamo_0.1.0_amd64.snap
sudo snap install --dangerous --classic biodynamo_0.1.0_amd64.snap
```

### Mac OS

``` sh
brew install Biodynamo/biodynamo/biodynamo
```


## Development Version

If you want to make changes to BioDynaMo itself, then you need to install the
development version.

### Ubuntu

```sh
git clone https://github.com/BioDynaMo/biodynamo
cd biodynamo

sudo script/install_prerequesites_ubuntu_16.04.sh

# source biodynamo environment
. /opt/biodynamo/biodynamo_dev.env

mkdir build && cd build
cmake .. && make -j4
sudo make install
```
