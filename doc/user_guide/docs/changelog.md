# Changelog

## 12.06.2018 ``

Change github option for `biodynamo new`. Previously it was opt-out (`--no-github`).
This commit changes it to opt-in (`--github`).

| Old                                 | New                                    |
| ----------------------------------- | -------------------------------------- |
| `biodynamo new --no-github`         | `biodynamo new` |
| `biodynamo new`                     | `biodynamo new --github` |

## 11.06.2018 `82e7e15`

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
