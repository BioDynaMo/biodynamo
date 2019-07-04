---
title: "Installation"
meta_title: "install"
keywords:
  -install
sidebar: "userguide"
---

<!--# Installation-->

To install BioDynaMo for the first time execute the following commands.
The installation will also install all required packages including ParaView, ROOT and Qt5.

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
./install.sh
```
<br>

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Important<b><h4>
    	<p>1. After the installation you need to restart your terminal.
	      In every new terminal execute <code>source <path-to-bdm-installation>/biodynamo-env.sh</code>
	      to use BioDynaMo!<br>
      2. It is also possible to use the library without running `make install`.
        You will just need to source `thisbdm.sh` from the build directory: `source
        <path-to-bdm-build-dir>/bin/thisbdm.sh`
	    3. BioDynaMo uses a customized version of ParaView.
		     Therefore, you should not install ParaView separately.
		</p>
    </div>
</a>

## Update Installation

The following commands update your BioDynaMo installation:

```bash
cd path/to/biodynamo
# make sure you are on the master branch
git checkout master
# get latest changes
git pull origin master
./install.sh
```

## Supported platforms

*  **Ubuntu 16.04 (recommended)**, 18.04
*  CentOS 7 (latest)
*  Mac OSX
