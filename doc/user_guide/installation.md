---
title: "Installation"
date: "2019-01-01"
path: "/docs/userguide/installation/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the installation page."
toc: true
image: ""
next:
    url:  "/docs/userguide/installation/"
    title: "Installation"
    description: "This is the installation page."
sidebar: "userguide"
keywords:
  -install
  -start
---

To install BioDynaMo for the first time execute the following command.
The installation will also install all required packages including ParaView, ROOT and Qt5.

```bash
curl https://biodynamo.org/install | bash
```
<br/>

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Important</b></h4>
    	<p>1. In every new terminal execute <code>source &lt;path-to-bdm-installation&gt;/bin/thisbdm.sh</code>
	      to use BioDynaMo!<br>
      2. It is also possible to use BioDynaMo without running <code>make install</code>.
        You will just need to source <code>thisbdm.sh</code> from the build directory: <code>`source &lt;path-to-bdm-build-dir&gt;/bin/thisbdm.sh`</code><br>
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
