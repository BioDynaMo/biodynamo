# Changelog

## 22.06.2018: Commit-Id

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
