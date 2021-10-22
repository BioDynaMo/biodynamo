---
title: "Introduction"
date: "2019-01-01"
path: "/docs/userguide/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the introduction page for the user guide."
toc: true
image: ""
next:
    url:  "/docs/userguide/"
    title: "Introduction"
    description: "This is the introduction page for the user guide."
sidebar: "userguide"
keywords:
  -introduction
  -intro
  -start
  -begin
  -beginning
---

## What is BioDynaMo?

BioDynaMo is a platform through which life scientists
can easily create, run, and visualise three-dimensional
biological simulations. Built on top of the latest computing
technologies, the BioDynaMo platform will enable users to
perform simulations of previously unachievable scale and
complexity, making it possible to tackle challenging scientific
research questions.


## Important Notes

### Version

This guide corresponds to the latest version of BioDynaMo. Please make sure that
your installation has the exact same version string as indicated at the top of the
[API documentation](/api/).
Please head over to our [installation/update instructions](/docs/userguide/installation) if you haven't installed
BioDynaMo yet, or the installation is outdated.

Execute `biodynamo --version` to check which version is installed on your system.
The version string will look like: `v1.01.86-6e7b5441`. This is how you can interpret
it: `vMAJOR.MINOR.PATCH-SHA`.
`SHA` is the git commit hash that can be used to check out the exact BioDynaMo version with `git checkout SHA`.

### Unstable API

BioDynaMo is in an early development stage. Therefore, our API changes quite
rapidly as we learn new requirements from our users. In case your simulation
does not compile after an update, please have a look at our [changelog](/docs/userguide/changelog)
and update your code.
