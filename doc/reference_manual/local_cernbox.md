# Speed up Installation Tests with a Local CERNBox Copy

The installation scripts fetch large precompiled dependencies like paraview
or root from CERNBox. To enable faster builds you can download the whole
CERNBox and tell BioDynaMo to access the local version. This is done with the
environmental flag `BDM_LOCAL_CERNBOX`. Use an absolute path to the directory
that contains the local copy.

```
export BDM_LOCAL_CERNBOX=/path/to/local/cernbox
```

NOTA BENE: At the moment there is no check if the local copy is in synch with
CERNBox. You have to ensure that yourself!

If you want to download the files from `cernbox.cern.ch` again execute:

```
unset BDM_LOCAL_CERNBOX
```
