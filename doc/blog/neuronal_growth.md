---
title: "Modelling Neuronal Growth in BioDynaMo"
date: "2019-10-01"
path: "/blog/neuronal_growth/"
meta_description: ""
---

The central nervous system is a vast and complex assembly of neurons responsible for information integration, resulting in an astonishing variety of phenotypic responses. Understanding how this highly complex structure emerges from a few progenitor cells that self-organize represents one of the biggest challenge to modern neuroscience.
To reach this goal, computational neuroscience and modelling have become an increasingly used tool during the past decades.

By being able to model both cell bodies and neurites (dendrites and axons) growth self organisation, BioDynaMo is particularly well suited to conduct neural tissue developmental studies.
By representing neurites as a chain of small cylinders attached to a cell body, it is possible to precisely model dendritic and axonal arbours. Validation can be done by comparing these simulated neurons with real morphologies obtained during in-vitro experiments. Metrics such as the arbour length and diameter, the number of branching point or the tortuosity of the branches can be easily calculated and used for validation.
Therefore, it is possible to investigate various aspects of neural development, such as the biological requirements to grow realistic morphologies, or the causes of abnormal development and potential clinical solutions.

Moreover, BioDynaMo is able to take into account interactions between neurons during development. This is particularly important as neuron-neuron interactions are known to play a crucial role during the development of the nervous system, and particularly during the development of neurites. Simulations can recapitulate contact mediated interactions and chemical communication between numerous cells and their neurites. This allow for instance the modelling of dendritic competition for space, a phenomenon observed notably in the retina, where dendritic arbours of the same cell type tend not to overlap.

Precisely simulating the development and interactions of a large number of neurons and their dendritic arbours (i.e. millions or billions of agents) can be extremely computationally intensive. However, BioDynaMo's high performance execution engine allows us to conduct such large scale simulations by taking advantage of modern computing techniques.

All in all, we expect BioDynaMo to be applied to various research topics in neuroscience.


Jean de Montigny
