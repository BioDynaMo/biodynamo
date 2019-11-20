---
title: "Modelling Thermodynamics in BioDynamo"
date: "2019-10-01"
path: "/blog/thermodynamics/"
meta_description: ""
---

BioDynaMo provides an excellent basis for the modelling of cells responding to cooling and thawing during cryopreservation, this is due to BioDynaMo’s ability to create agent based simulations where by each individual cell can act independently of those around it based on local environmental factors.

Currently through the use of biology modules within BioDynaMo, one can simulate a cells osmotic response to changes in temperature for 0°C and below for multiple cooling rates.

This is modelled as a change in the volume of the cell, which can then be interpreted by the chemical diffusion grid as water diffusing into the extracellular medium , in the case of cooling, and increase the concentration around the cell accordingly. The concentration grid will then diffuse the water throughout the remaining extracellular space at each time step of the simulation.

At current there is a Thermodynamics module being developed to accompany the biology modules for cellular behaviour and chemical diffusion, which will allow for further simulations to have a dynamic temperature field alongside the already existing dynamic chemical diffusion grid.

