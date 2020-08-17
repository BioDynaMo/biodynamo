---
title: "Using BioDynaMo to Study Virus Spread in Closed Environments"
date: "2020-08-07"
path: "/blog/epidemiology/"
meta_description: ""
---

In addition to cellular studies (tumour growth modelling or neural development), BioDynaMo can be used to simulate different systems, such as groups of people and their interactions. As BioDynaMo has been developed to simulate billions of cells, it should be able to handle fairly easily simulations of up to millions of people.

In one of these types of simulation, BioDynaMo is used to study the spreading of viruses in indoor spaces, specifically COVID-19 in droplets and aerosols. We are investigating several scenarios, such as public transportation (bus, metro) and buildings (supermarkets, offices). In these simulations BioDynaMo is in charge of simulating the behaviour and characteristics of individuals, while the [ROOT](http://root.cern) geometrical modeller is used to define the precise environmental geometry. Each individual can then independently move around in these environments where infected individuals can possibly contaminate healthy ones through the spreading of droplets and aerosols. By studying different geometries, airflows, distancing, masks and other parameters we can hopefully determine which environments are best to avoid virus buildup and prevent people from getting infected.

We are working closely together with the epidemiological department of the Univeristy of Geneva to make sure that our simulations reflect correctly the many observed cases of virus outbreaks in closed spaces.

Follow the progress of this research in a future blog post.


Jean de Montigny and Fons Rademakers