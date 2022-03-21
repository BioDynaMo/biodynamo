---
title: "Singularity"
date: "2022-02-15"
path: "/docs/userguide/singularity/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the singularity page."
toc: true
image: ""
next:
    url:  "/docs/userguide/singularity/"
    title: "Singularity"
    description: "This is the diffusion page."
sidebar: "userguide"
keywords:
  -Singularity
  -HPC
  -performance
---
In addition to running BioDynaMo on standard computers, users can also utilise the BioDynaMo frame work on high 
performance computers (HPCs). Due to many HPCs having higher security than standard computers however it is likely
that a user wishing to do this will need to build BioDynaMo within a container on the chosen HPC.

For this we have chosen to Singularity. Singularity is an open source programme for operating-system-level virtualization. 
Singularity works simillar to other popular container programmes such as Docker. However, due to its higher level of 
security it has seen more widespread uptake by HPCs and we thus beleive the best choise for users.

To create our singularity image for BioDynaMo we can use on of two methods.

## Method 1, directly building on the HPC.
For the first method, we directly import the biodynamo Singularity file from our local PC to the desired HPC as follows.  
Firstly, we must scp into the desired HPC from our local PC. This can be done directly through the terminal as follows.
```bash
ssh -x [HPC.ADDRESS]
```
We then must secure copy the BioDynaMo singularity file from the chosen PC to the local HPC.
```bash
scp [user@]SRC_LOCALPC/SingularityFileLocation:]Singularity ~/
```
From here we can build BioDynaMo utilising the fake root feature of singularity. 
```bash
singularity build --fakeroot Singularity.sif Singularity
```
Fakeroot is not needed if you have sudo rights on the HPC but this is highly unlikely for most individuals.
In that case however we simply amend the above to:
```bash
sudo singularity build Singularity.sif Singularity
```
Then simply run the BioDynaMo singularity container using:
```bash
singularity run Singularity.sif
```
## Method 2, Export image from local PC to HPC.
If you cannot utilise either sudo or fakeroot on the HPC, we can isntead build the BioDynaMo image on our local PC and export the image.
This works simillar to method 1 but with a change in order. Firstly, we must build our image on the local PC with sudo rights:
```bash
sudo singularity build Singularity.sif Singularity
```
Then ssh into the HPC.
```bash
ssh -x [HPC.ADDRESS]
```
secure copy the image from our PC to the local HPC.
```bash
scp [user@]SRC_LOCALPC/SingularityFileLocation:]Singularity.sif ~/
```
Finally we once again run the Singularity image on the HPC:
```bash
singularity run Singularity.sif
```

If you wish to read further about Singularity, you can find a substantial amount of information on the 
singularity home website: https://sylabs.io/guides/3.5/user-guide/introduction.html. Including many tutorials and more 
complex use cases.

