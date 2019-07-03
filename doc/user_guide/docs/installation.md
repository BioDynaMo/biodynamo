# Installation

To install BioDynaMo for the first time execute the following commands.
The installation will also install all required packages including ParaView, ROOT and Qt5.

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
./install.sh
```
<<<<<<< f519d7fb9d2af4997af4e99811bed30a0ec5dc5a

!!! important

    1. After the installation you need to restart your terminal.
      In every new terminal execute `source <path-to-bdm-installation>/bin/thisbdm.sh`
      to use BioDynaMo!

    2. It is also possible to use the library without running `make install`. You will just need to source `thisbdm.sh` from the build
       directory: `source <path-to-bdm-build-dir>/bin/thisbdm.sh`

    3. BioDynaMo uses a customized version of ParaView.
	     Therefore, you should not install ParaView separately.
=======
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
>>>>>>> Updated .md for Gatsby format

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
