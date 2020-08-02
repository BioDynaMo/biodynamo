---
title: "Changelog"
date: "2019-01-01"
path: "/docs/userguide/changelog/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the changelog page."
toc: true
image: ""
next:
    url:  "/docs/userguide/changelog/"
    title: "Changelog"
    description: "This is the changelog page."
sidebar: "userguide"
keywords:
  -changelog
  -change
  -log
  -track
  -tracking
---
## 02.09.2020 [`4cca726`](https://github.com/BioDynaMo/biodynamo/commit/4cca726f9de686501527456e30df9c87bc06d812)

Change biodynamo.org/bioapi to biodynamo.org/api

## 11.02.2020 [`0d6407d`](https://github.com/BioDynaMo/biodynamo/commit/0d6407d52e8fdfdf981364ed6c41aa37206f422e)

Upgrade to Python 3.

Since Python 2 is not supported in 2020 anymore, we upgrade to Python 3 (Mac OS
excluded momentarily). We rely on PyEnv to create a consistent Python 3
environment through `thisbdm.sh`. For those who are not on Mac OS, please make
sure that PyEnv is installed correctly by rerunning the prerequisites script, or
check out the installation guide.

## 11.09.2019 [`0719cd6`](https://github.com/BioDynaMo/biodynamo/commit/0719cd6bafd100439336de71bee55ee2c506e9cf)

Add SBML integration.
Currently SBML is not supported on MacOS (see https://trello.com/c/vKPbh4iG).

## 05.08.2019 [`d7bab00`](https://github.com/BioDynaMo/biodynamo/commit/d7bab00d9307b29794490de0519c4c5c77bac435)

Major improvements of the BioDynaMo's build system.

  * Refactor build procedure.
  * Improve dependency detection and diagnostic messages for the user.
  * Rename `biodynamo-env.sh` to `thisbdm.sh`.
  * BioDynaMo can be used directly from the build directory. The install step has become optional.
  * Improve user and developer guide.

## 20.06.2019 [`257f1a3`](https://github.com/BioDynaMo/biodynamo/commit/257f1a39ba501da9915c5a50c93d517ac2673919)

Add support for multiscale simulations.

## 14.06.2019 [`cb15679`](https://github.com/BioDynaMo/biodynamo/commit/cb1567947595090e3b43a3a44bf74477699d83b0)

Release BioDynaMo dynamic.

This is a more user-friendly version of BioDynaMo. It removes most template code, and does not require compile time parameters, resulting in a simpler API. However, this comes at a small cost in performance. In due time we hope to regain the lost performance.

There are too many small API changes to present an exhaustive list here.
We recommend to have a look at the
[demo folder](https://github.com/BioDynaMo/biodynamo/tree/cb1567947595090e3b43a3a44bf74477699d83b0/demo)
folder or directly inspect the
[changes](https://github.com/BioDynaMo/biodynamo/commit/cb1567947595090e3b43a3a44bf74477699d83b0#diff-d61c94874975bac07d55c551632c6e1c)
of the demo folder.

## 31.01.2019 [`3a51e76`](https://github.com/BioDynaMo/biodynamo/commit/3a51e76fa109cee10e11776a92bd4ce3b299ee93)

Improve file structure in directory src/ and test/unit

**API changes**

| Old                | New         |
| ------------------ | ----------- |
| `SimulationObject` | `SimObject` |

## 21.01.2019 [`1968ec2`](https://github.com/BioDynaMo/biodynamo/commit/1968ec2969b3fa38e6856d96f6e7294c36e71634)

This commit introduces a series of changes to encapsulate different discretization
strategies. Other modifications have been made along the way to facilitate this
change and solve known issues.

Discretization governs three main questions:

-   When should new simulation objects be visible?
-   When should simulation objects be removed from the simulation?
-   If a simulation object is updated, when should the change be visible?
-   Should operations observe the values from the last iteration or from the
    previous operation?

Since simulations might have different requirements, this commit introduces
execution contexts to define and encapsulate this behavior in one place.

**First**, this commit introduces unique ids for simulation objects that stay constant
during the whole simulation and are not reused if a simulation object is removed
from the simulation. Among other things, this improves debugging simulations.

**Second**, building upon the introduction of unique ids, manual updates of `SoPointer`
(references to another simulation object--e.g. `NeuriteElement::daughter_left_`)
becomes obsolete. This is now managed by the `ResourceManager`.

**Third**, this commit adds an in-place execution context.

-   Simulation objects that are added or removed are visible to the whole simulation
    after the next timestep.
-   Operations directly modify simulation objects inside the `ResourceManager`.
    Thus, the result depends on the order in which sim objects are updated.
    Operations (biology modules or mechanical interactions) see the updated
    values from the previous operation.

**Forth**, this commit solves two race condition issues:

-   Adding new simulation objects to the `ResourceManager` caused issues if it
    triggered a growth. References and pointers to simulation objects were
    invalidated during this operation.
-   Modifications of neighbors. Two threads could potentially update the same
    neighbor.

**Fifth**, result from mechanical interactions will change. Up to now the implementation
was inconsistent with respect to when updates will take effect. Biology modules
were updated in place, while results from mechanical interactions where cached
and applied once all simulation objects have been updated. Now, this behavior is the
responsibility of the execution context. In case of the `InPlaceExecutionContext` this means
that during iteration `t` some cells observe neighbors that have already been updated
to timestep `t'`.

**API changes**

Several API changes were necessary to implement the described functionality.
A general rule is to use the new execution context to perform actions instead of
using the `ResourceManager`, or `Grid` directly. The thread local execution context
can be obtained from the simulation object (e.g. calling `sim->GetExecutionContext()`).

Exemplary API changes:

-   Method `ResourceManager::New` was removed. During a simulation only use
    e.g. `InPlaceExecutionContext::New`. During setup of the initial model using
    `ResourceManager::push_back` is also fine.
-   Method `ResourceManager::Get` has been changed to return a const pointer.
    Thus `rm->Get<Cell>()->push_back(new_cell)` won't work anymore. However, calling
    `rm->Get<Cell>()->size()` is still fine.

For the full set of changes that are visible to the user, it is best to have a
look at the demo folder and the differences of [this commit](https://github.com/BioDynaMo/biodynamo/commit/1968ec2969b3fa38e6856d96f6e7294c36e71634).

## 25.10.2018 [`b197542`](https://github.com/BioDynaMo/biodynamo/commit/b197542ef90864c97af899baa8b1ca6d68c71ef7)

Resolve ROOT-9321 by removing TBase template parameter of simulation objects

Motivation:

-   Workaround for ROOT-9321
-   Shortens full name of simulation objects

Move duplicated biology module code from `Cell` and `NeuriteELement`
to `SimulationObject`

This change requires a different signature of `BDM_SIM_OBJECT_HEADER`.

1.  Remove the suffix `Ext` from the first parameter
2.  Add the base class name as a second parameter.

In other words, copy the parameters from `BDM_SIM_OBJECT` to the beginning of `BDM_SIM_OBJECT_HEADER`

```cpp
class Cell : public SimulationObject {
  BDM_SIM_OBJECT_HEADER(Cell, SimulationObject, 1, ...)
```

| Old                                      | New                                                            |
| ---------------------------------------- | -------------------------------------------------------------- |
| `BDM_SIM_OBJECT_HEADER(CellExt, 1, ...)` | `BDM_SIM_OBJECT_HEADER(Cell,`**`SimulationObject`**`, 1, ...)` |

## 08.10.2018 [`8a97cf2`](https://github.com/BioDynaMo/biodynamo/commit/8a97cf21ad3e07be19f764d116eb10cae5c6ab05)

Allow builds without dictionaries to speed up compile time

Early development of a simulation requires fast iteration cycles.
During this stage serialization features are not needed. Thus,
we support builds without dictionaries.

By default dictionaries will be built. To turn them off, run:
cmake -Ddict=off ..

| Old             | New                   |
| --------------- | --------------------- |
| ClassDef(...)   | BDM_CLASS_DEF(...)    |
| ClassDefNV(...) | BDM_CLASS_DEF_NV(...) |

## 18.09.2018 [`3a380e4`](https://github.com/BioDynaMo/biodynamo/commit/3a380e451ed1d691a6b8dce4c46d82e7faaf5ddc)

Refactor [parameters](/docs/userguide/parameter).

-   Add functionality to define compile time parameters for a specific simulation
    object. This was necessary due to compile time errors of neurite biology modules.
    (Although they were not used for neurons, the compiler tried to compile them)
    This replaces the reinterpret cast workaround.
-   `Simulation::GetActive()->GetParam()` returns const pointer
    Runtime parameter should not be changed during the simulation. This simplifies
    the distributed runtime.
-   Add macros to simplify definition of compile time parameter.
-   All compile time parameter that take more than one type will be defined using
    `CTList`. No more distinction between `Variant` and `VariadicTypedef`.
-   Improve modularity. Each module can have its own parameter class.
    All parameter classes will be combined into `CompileTimeParam::Param`.
-   Make all runtime parameters non static.
-   Rename `AtomicTypes` to `SimObjectTypes`

Please have a look at the changes of the demos to
see which lines need to be changed in your simulation after [this commit](https://github.com/BioDynaMo/biodynamo/commit/3a380e451ed1d691a6b8dce4c46d82e7faaf5ddc).

## 29.08.2018 [`a373fca`](https://github.com/BioDynaMo/biodynamo/commit/a373fcaad5b50d2ec8ad5a9d8a218adf7850fcc6)

Add the concept of [Events](/docs/userguide/event).
This is an important change to support extensibility. Now, there is a clear
way to tell BioDynaMo what should happen with a new data member for a specific
event.

| Old                             | New                                                                                                                            |
| ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| `gAllBmEvents`                  | `gAllEventIds`                                                                                                                 |
| `gNullEvent`                    | `gNullEventId`                                                                                                                 |
| `gCellDivision`                 | `CellDivisionEvent::kEventId`                                                                                                  |
| `gNeuriteElongation`            | `SplitNeuriteElementEvent::kEventId`                                                                                           |
| `gNeuriteBranching`             | `NeuriteBranchingEvent::kEventId`                                                                                              |
| `gNeuriteBifurcation`           | `NeuriteBifurcationEvent::kEventId`                                                                                            |
| `gNeuriteSideCylinderExtension` | `SideNeuriteExtensionEvent::kEventId`                                                                                          |
|                                 | Simulation objects and biology modules must have an event constructor and event handler function in order to support an event. |

## 16.07.2018 [`3bac827`](https://github.com/BioDynaMo/biodynamo/commit/3bac827b53b5f9f1066867f89bb47e9f69f90b0a)

Change github option for `biodynamo new`. Previously it was opt-out (`--no-github`).
This commit changes it to opt-in (`--github`).

| Old                         | New                      |
| --------------------------- | ------------------------ |
| `biodynamo new --no-github` | `biodynamo new`          |
| `biodynamo new`             | `biodynamo new --github` |

## 11.07.2018 [`82e7e15`](https://github.com/BioDynaMo/biodynamo/commit/82e7e15463e2e7dfdb488073a01fd563305fc26c)

-   Add `biodynamo demo` command to try out the demos
    `biodynamo demo` lists all the available demos
    `biodynamo demo <demo-name> <dir>` sets up the demo `<demo-name>` in directory `<dir>`.
    If `<dir>` is not specified, it defaults to the current working directory.

## 26.06.2018 [`ba4fe1f`](https://github.com/BioDynaMo/biodynamo/commit/ba4fe1f9f911e007bd61237d53a8399aa1b2268d#comments)

-   Add support for multiple simulations per process. Only one simulation
    can be active at the same time. Introduces new class `Simulation` (see [API](/api/classbdm_1_1Simulation.html)).
    This change causes many API changes -- see subsection below.

-   Write simulation files to separate directory: `output/simulation-id/`

-   Integrate simulation template for `biodynamo new` into the biodynamo repository to avoid
    inconsistencies with the biodynamo version.

| Old                                                      | New                                                                   |
| -------------------------------------------------------- | --------------------------------------------------------------------- |
| `InitializeBioDynaMo(...)`                               | `Simulation simulation(...)`                                          |
| `Rm()` <br> `TRm::Get()` <br>  `TResourceManager::Get()` | `auto* rm = simulation.GetResourceManager();`                         |
| `GetDiffusionGrid(...)`                                  | `rm->GetDiffusionGrid(...)`                                           |
| `Grid::GetInstance()`                                    | `auto* grid = simulation.GetGrid();`                                  |
| `Param::some_parameter_;`                                | `auto* param = simulation.GetParam();` <br> `param->some_parameter_;` |
