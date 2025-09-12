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

To install and update BioDynaMo execute the following command.

```bash
curl https://biodynamo.github.io/install | bash
```

This will also install all prerequisites, including ROOT, ParaView and Qt5.
By default the installation directory is set to `$HOME/biodynamo-vX.Y.Z`, where X.Y.Z is the version number.

<br/>
<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Important</b></h4>
    	<p>In every new terminal execute <code>source &lt;installation-directory&gt;/bin/thisbdm.sh</code>
      to use BioDynaMo, where <code>&lt;installation-directory&gt;</code> is the BioDynaMo installation directory.<br>To do this automatically for every shell, put this in your <code>.bashrc</code> or <code>.zshrc</code>.<br>
        </p>
    </div>
</a>

## Install development version

The command shown at the top of the page installs the [latest BioDynaMo release](https://github.com/BioDynaMo/biodynamo/releases/latest) including all patches for this release.

If you want to install the latest development version with all experimental features, use:

```bash
export BDM_INSTALL=master
curl https://biodynamo.github.io/install | bash
```

## Install specific version

It is also possible to install a specific BioDynaMo version.
Let's assume that in a publication you read that the authors used BioDynaMo version `v1.01.115-e1088d4a`.
To install the exact same version on your system, use the following commands.

```bash
export BDM_INSTALL=e1088d4a
curl https://biodynamo.github.io/install | bash
```

## Supported platforms

*  Ubuntu 20.04, 22.04, 24.04
*  MacOS > 12.0 (Intel and ARM)

Currently, we do **not** support Windows or Windows subsystem for Linux.

<br/>
<a class="sbox" href= "/docs/devguide/build/" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Note</b></h4>
    	<p>If you are a developer please follow the build instructions in our <font color="blue"><u>Developer Guide</u></font>.
        </p>
    </div>
</a>
<br>


