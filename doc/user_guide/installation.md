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

To install BioDynaMo execute the following command.

```bash
curl https://biodynamo.org/install | bash
```

This will also install all prerequisites, including ParaView, ROOT and Qt5.
By default the installation directory is set to `$HOME/biodynamo-vX.Y.Z`, where X.Y.Z is the version number.

<br/>

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Important</b></h4>
    	<p>In every new terminal execute <code>source $BDMSYS/bin/thisbdm.sh</code> to use BioDynaMo!<br>
		</p>
    </div>
</a>

## Update Installation

The following command updates your BioDynaMo installation:

```bash
curl https://biodynamo.org/install | bash
```

## Supported platforms

*  Ubuntu 16.04, 18.04, 20.04
*  CentOS 7
*  Mac OSX
