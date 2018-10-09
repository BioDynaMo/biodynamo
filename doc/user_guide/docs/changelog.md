# Changelog

## 08.10.2018 [`8a97cf2`](https://github.com/BioDynaMo/biodynamo/commit/8a97cf21ad3e07be19f764d116eb10cae5c6ab05)

Allow builds without dictionaries to speed up compile time                                 

Early development of a simulation requires fast iteration cycles.                  
During this stage serialization features are not needed. Thus,                                  
we support builds without dictionaries.                                     

By default dictionaries will be built. To turn them off, run:                                   
cmake -Ddict=off ..

| Old                                 | New                                    |
| ----------------------------------- | -------------------------------------- |
| ClassDef(...)                       | BDM_CLASS_DEF(...)                     |
| ClassDefNV(...)                     | BDM_CLASS_DEF_NV(...)                  |

## 18.09.2018 [`3a380e4`](https://github.com/BioDynaMo/biodynamo/commit/3a380e451ed1d691a6b8dce4c46d82e7faaf5ddc)

Refactor [parameters](parameter).

*  Add functionality to define compile time parameters for a specific simulation
   object. This was necessary due to compile time errors of neurite biology modules.
   (Although they were not used for neurons, the compiler tried to compile them)
   This replaces the reinterpret cast workaround.
*  `Simulation<>::GetActive()->GetParam()` returns const pointer
   Runtime parameter should not be changed during the simulation. This simplifies
   the distributed runtime.
*  Add macros to simplify definition of compile time parameter.
*  All compile time parameter that take more than one type will be defined using
   `CTList`. No more distinction between `Variant` and `VariadicTypedef`.
*  Improve modularity. Each module can have its own parameter class.
   All parameter classes will be combined into `CompileTimeParam::Param`.
*  Make all runtime parameters non static.
*  Rename `AtomicTypes` to `SimObjectTypes`

Please have a look at the changes of the demos to
see which lines need to be changed in your simulation after [this commit](https://github.com/BioDynaMo/biodynamo/commit/3a380e451ed1d691a6b8dce4c46d82e7faaf5ddc).

## 29.08.2018 [`a373fca`](https://github.com/BioDynaMo/biodynamo/commit/a373fcaad5b50d2ec8ad5a9d8a218adf7850fcc6)

Add the concept of [Events](event).
This is an important change to support extensibility. Now, there is a clear
way to tell BioDynaMo what should happen with a new data member for a specific
event.

| Old                                 | New                                    |
| ----------------------------------- | -------------------------------------- |
| `gAllBmEvents`                      | `gAllEventIds` |
| `gNullEvent`                        | `gNullEventId` |
| `gCellDivision`                     | `CellDivisionEvent::kEventId` |
| `gNeuriteElongation`                | `SplitNeuriteElementEvent::kEventId` |
| `gNeuriteBranching`                 | `NeuriteBranchingEvent::kEventId` |
| `gNeuriteBifurcation`               | `NeuriteBifurcationEvent::kEventId` |
| `gNeuriteSideCylinderExtension`     | `SideNeuriteExtensionEvent::kEventId` |
|                                     | Simulation objects and biology modules must have an event constructor and event handler function in order to support an event. |


## 16.07.2018 `3bac827`

Change github option for `biodynamo new`. Previously it was opt-out (`--no-github`).
This commit changes it to opt-in (`--github`).

| Old                                 | New                                    |
| ----------------------------------- | -------------------------------------- |
| `biodynamo new --no-github`         | `biodynamo new` |
| `biodynamo new`                     | `biodynamo new --github` |

## 11.07.2018 `82e7e15`

* Add `biodynamo demo` command to try out the demos
`biodynamo demo` lists all the available demos
`biodynamo demo <demo-name> <dir>` sets up the demo `<demo-name>` in directory `<dir>`.
If `<dir>` is not specified, it defaults to the current working directory.

## 26.06.2018 `ba4fe1f`

* Add support for multiple simulations per process. Only one simulation
can be active at the same time. Introduces new class `Simulation` (see [API](https://biodynamo.github.io/api/structbdm_1_1Simulation.html)).
This change causes many API changes -- see subsection below.

* Write simulation files to separate directory: `output/simulation-id/`

* Integrate simulation template for `biodynamo new` into the biodynamo repository to avoid
inconsistencies with the biodynamo version.

### API changes

| Old                                 | New                                    |
| ----------------------------------- | -------------------------------------- |
| `InitializeBioDynaMo(...)`          | `Simulation<> simulation(...)` |
| `Rm()` <br> `TRm::Get()` <br>  `TResourceManager::Get()` | `auto* rm = simulation.GetResourceManager();` |
| `GetDiffusionGrid(...)`             | `rm->GetDiffusionGrid(...)` |
| `Grid::GetInstance()`               | `auto* grid = simulation.GetGrid();` |
| `Param::some_parameter_;`           | `auto* param = simulation.GetParam();` <br> `param->some_parameter_;` |
