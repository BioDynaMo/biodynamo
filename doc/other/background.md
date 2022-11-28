---
title: "Background"
date: "2019-10-01"
path: "/background/"
meta_description: ""
---

Biological systems are built in a way that allows for extremely high complexity, robustness and performance. For instance, the human brain is the most complex entity known to us, and outperforms current computing technology in many respects. Nevertheless, it arises through a well-orchestrated developmental process from a single precursor cell, the zygote. How does this marvelous process occur, producing the seat of our thoughts, emotions and plans? How do cells interact, communicate and self-organize into complex organisms?

Computer simulation serves as a very powerful tool for modeling and helping understand dynamics within such complex biological systems. It allows devising models, formulating new hypotheses and predictions, as well as conducting computational “experiments” in very efficient and inexpensive ways.  Moreover, it enables scientists to collaboratively bring together huge amounts of experimental results and previous knowledge within a unified, comprehensive framework. However, it is usually very challenging to conduct such computer simulations, because very detailed, dynamical mechanisms and large-scale systems have to be accounted for (e.g. physical forces, reaction-diffusion processes, gene regulatory dynamics, etc.). Hence, these models comprise very different spatial and temporal scales (e.g. ranging from milliseconds to weeks), and so require extremely sophisticated computational resources.

In order to help biologists and interdisciplinary researchers tackle these challenges, we are working on a new platform called BioDynaMo, to model biological dynamics in various contexts. This software allows simulating a wide range of biological processes, such as cell proliferation, neurite growth and electrical activity, by exploiting hybrid cloud computer systems in a very efficient manner.

To demonstrate the power and uniqueness of BioDynaMo, simulations of human brain development will be conducted in the context of the Human Brain Development Project. However, the same software is applicable also to very different domains. In the following sections, potential topics where BioDynaMo could be applied are explained.

From a neuroscience point of view, computational models have been strongly used to investigate aspects of neural development. One crucial advantage of this computational approach is that it allows addressing the question of how neuronal structure and function develops from a very simple initial setting, by genetic encoding as well as intercellular interactions. In particular, the recent advances in computing technology have made it possible to take into account various factors (e.g. genetics, physical interactions, external stimuli, etc.) that are relevant during neural development. First steps have been made into this direction of the computational modeling of neural development (1-4). Using high-performance cloud computing, we plan to apply BioDynaMo for simulating large-scale models of the human brain. Our aim is to use BioDynaMo to go push the limits beyond what is currently possible with regard to neurodevelopmental simulations. In particular, we want to investigate the influence of electrical activity on brain development, and apply these insights to simulate realistic large-scale systems with neuron numbers that are comparable to those of the human brain.

The modeling of biological dynamics is also crucial in cancer research. The growth of brain tumors is a very complex process involving many different developmental processes, such as genetics, chemical signaling, interactions with the vascular system, etc. Quantitative approaches based on computer simulations allow modeling these complex processes, to gain a mechanistic understanding and even yield predictions on clinical progression (5, 6). A computer simulation of a tumor in cortical tissue can be found here. In this example, a single tumor cell proliferates and grows continuously, producing daughter cells that also divide in an uncontrolled manner. This process gives rise to a steadily growing cancer that comprises multiple satellite tumors (black). Interestingly, such complex, non-spherical structures develop because small variations in the physical interactions of the tumor with the immediate local environment can rapidly give rise to non-isotropic growth, leading to the complex structure seen at the end of the video. This simulation was conducted using a parallelized version of the software framework Cx3D, which shares many of the scientific concepts that are fundamental to BioDynaMo.

Computer simulations of detailed biological dynamics are used also for many other topics, such as for example biofilm formation (7), tissue growth (8), plant growth (9), microbial systems (10) or cryopreservation (11). By taking inspiration from how nature constructs entities, and leveraging the power of technological advances, we envisage BioDynaMo to become a standard platform for research on a very large number of biologically relevant topics.

References

1.         Bauer R, Zubler F, Hauri A, Muir DR, Douglas RJ. Developmental origin of patchy axonal connectivity in the neocortex: a computational model. Cereb Cortex. 2014;24(2):487-500.

2.         Bauer R, Zubler F, Pfister S, Hauri A, Pfeiffer M, Muir DR, et al. Developmental self-construction and -configuration of functional neocortical neuronal networks. PLoS Comput Biol. 2014;10(12):e1003994.

3.         Zubler F, Hauri A, Pfister S, Bauer R, Anderson JC, Whatley AM, et al. Simulating cortical development as a self constructing process: a novel multi-scale approach combining molecular and physical aspects. PLoS Comput Biol. 2013;9(8):e1003173.

4.         Koene RA, Tijms B, van Hees P, Postma F, de Ridder A, Ramakers GJ, et al. NETMORPH: a framework for the stochastic generation of large scale neuronal networks with real_tistic neuron morphologies. Neuroinformatics. 2009;7(3):195-210.

5.         Macklin P, Edgerton ME, Thompson AM, Cristini V. Patient-calibrated agent-based modelling of ductal carcinoma in situ (DCIS): from microscopic measurements to macroscopic predictions of clinical progression. J Theor Biol. 2012;301:122-40.

6.         Altrock PM, Liu LL, Michor F. The mathematics of cancer: integrating quantitative models. Nat Rev Cancer. 2015;15(12):730-45.

7.         Poplawski NJ, Shirinifard A, Swat M, Glazier JA. Simulation of single-species bacterial-biofilm growth using the Glazier-Graner-Hogeweg model and the CompuCell3D modeling environment. Math Biosci Eng. 2008;5(2):355-88.

8.         Swat MH, Thomas GL, Belmonte JM, Shirinifard A, Hmeljak D, Glazier JA. Multi-scale modeling of tissues using CompuCell3D. Methods Cell Biol. 2012;110:325-66.

9.         Mathieu A, Cournede PH, Letort V, Barthelemy D, de Reffye P. A dynamic model of plant growth with interactions between development and functional mechanisms to study plant structural plasticity related to trophic competition. Ann Bot. 2009;103(8):1173-86.

10.       Kang S, Kahan S, Momeni B. Simulating microbial community patterning using Biocellion. Methods Mol Biol. 2014;1151:233-53.

11.       Kashuba CM, Benson JD, Critser JK. Rationally optimzied cryopreservation of multiple mouse embryonic stem cell lines: II - Mathematical prediction and experimental validation of optimal cryopreservation protocols. Cryobiol. 2014;68(2):176-184.
