---
title: "BioDynaMo Notebooks"
date: "2019-11-11"
path: "/docs/userguide/notebook/"
meta_title: "BioDynaMo Dev Guide"
meta_description: "Information about BioDynaMo Notebooks"
toc: true
image: ""
next:
    url:  "/docs/userguide/add_page/"
    title: "Adding documentation webpage"
    description: "Information about adding documentation to the BioDynaMo website."
sidebar: "userguide"
keywords:
  -notebooks
  -jupyter
  -web-app
---

BioDynaMo supports the use of Jupyter Notebooks to create, run and visualize
simulations in. This is a more user-friendly interface to BioDynaMo, and is
intended mostly for educational and demonstrative purposes, as the performance
capabilities are limited.

## What is a Notebook?

Jupyter Notebook is a web application that allows you to interactively work with
code snippets. It allows you to write text (in Markdown and/or HTML format) to
give information about the contents of code snippets, or to give context.
Besides code snippets, you can embed visualizations (e.g. BioDynaMo visualizations),
and other forms of media (e.g. images, videos, etc.). A Notebook is composed of
"cells" with certain contents. There are different type of cells to contain
different types of content (e.g. code snippets, text).

You can use Notebooks to play around interactively with code snippets
(e.g. to test code, or to quickly change parameters). Another great use is
to create tutorials that can be presented to others for demonstrative or 
educational purposes. You can embed the visualization to bring across certain
points about your simulation in an effective manner.

Notebooks in BioDynaMo are supported by the ROOT framework. Since simulations
in BioDynaMo are currently written in C++, we make use of 'ROOT Notebooks' to
get the same functionalities one would expect in regular (Python-based) Jupyter
Notebooks.

## How to open a Notebook

After you have installed BioDynaMo, there are a couple of Notebooks already
available to play around with. They can be found in `$BDMSYS/notebooks`, and
are in the file format `.ipynb`. You can open them by executing the following
command in a terminal (where `thisbdm.sh` is sourced):

``` sh
root --notebook
```

This will automatically open up the Notebook web application in your default
browser. It should show a list of directories. Browse to your specific notebook,
and click on the `.ipynb` file. This should open the notebook.

## How to interact with a Notebook

The way to interact with a Notebook in BioDynaMo is almost exactly the same as
with regular Jupyter notebooks.

You can execute the context of a cell by selecting a cell by simply clicking on
it. Then from the menu bar open the "Cell" drop down menu and click on "Run Cells",
or simply "Ctrl + Enter" on your keyboard. If you wish to go through the cells
without having to select them individually you can also select the "Run Cells and Select Below"
option from the "Cell" drop down menu, or "Shift + Enter" on your keyboard.

If you wish to execute the entire notebook at once you can click on "Restart & Run All"
from the "Kernel" dropdown menu.

!!! Note
    You can find a list of common shortcuts in the "command palette" by clicking
    on the keyboard icon in the menu bar

## How to create a Notebook

If you are in the file browsing view of ROOT Notebooks, you can create a new
notebook by pressing the "New" button on the right top corner, and selecting
"ROOT C++". If you have already an existing Notebook open, you can achieve the
same with "File" >> "New Notebook" >> "ROOT C++".
