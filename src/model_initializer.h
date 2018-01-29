#ifndef MODEL_INITIALIZER_H_
#define MODEL_INITIALIZER_H_

#include <ctime>
#include <string>
#include <vector>

#include "diffusion_grid.h"
#include "random.h"
#include "resource_manager.h"

namespace bdm {

struct ModelInitializer {
  /// Creates a 3D cubic grid of simulation objects and adds them to the
  /// ResourceManager. Type of the simulation object is determined by the return
  /// type of parameter cell_builder.
  ///
  ///     ModelInitializer::Grid3D(8, 10, [](const std::array<double, 3>& pos){
  ///     return Cell(pos); });
  /// @param      cells_per_dim  number of simulation objects on each axis.
  ///                            Number of generated simulation objects =
  ///                            `cells_per_dim ^ 3`
  /// @param      space          space between the positions - e.g space = 10:
  ///                            positions = `{(0, 0, 0), (0, 0, 10), (0, 0,
  ///                            20), ... }`
  /// @param      cell_builder   function containing the logic to instantiate a
  ///                            new simulation object. Takes `const
  ///                            std::array<double, 3>&` as input parameter
  ///
  template <typename Function, typename TResourceManager = ResourceManager<>>
  static void Grid3D(size_t cells_per_dim, double space,
                     Function cell_builder) {
    auto rm = TResourceManager::Get();
    // Determine simulation object type which is returned by the cell_builder
    using FunctionReturnType = decltype(cell_builder({0, 0, 0}));

    auto container = rm->template Get<FunctionReturnType>();
    container->reserve(cells_per_dim * cells_per_dim * cells_per_dim);
    for (size_t x = 0; x < cells_per_dim; x++) {
      auto x_pos = x * space;
      for (size_t y = 0; y < cells_per_dim; y++) {
        auto y_pos = y * space;
        for (size_t z = 0; z < cells_per_dim; z++) {
          auto new_simulation_object = cell_builder({x_pos, y_pos, z * space});
          container->push_back(new_simulation_object);
        }
      }
    }
    container->Commit();
  }

  /// Creates a 3D grid of simulation objects and adds them to the
  /// ResourceManager. Type of the simulation object is determined by the return
  /// type of parameter cell_builder.
  ///
  ///     ModelInitializer::Grid3D({8,6,4}, 10, [](const std::array<double, 3>&
  ///     pos){ return Cell(pos); });
  /// @param      cells_per_dim  number of simulation objects on each axis.
  ///                            Number of generated simulation objects =
  ///                            `cells_per_dim[0] * cells_per_dim[1] *
  ///                            cells_per_dim[2]`
  /// @param      space          space between the positions - e.g space = 10:
  ///                            positions = `{(0, 0, 0), (0, 0, 10), (0, 0,
  ///                            20), ... }`
  /// @param      cell_builder   function containing the logic to instantiate a
  ///                            new simulation object. Takes `const
  ///                            std::array<double, 3>&` as input parameter
  ///
  template <typename Function, typename TResourceManager = ResourceManager<>>
  static void Grid3D(const std::array<size_t, 3>& cells_per_dim, double space,
                     Function cell_builder) {
    auto rm = TResourceManager::Get();
    // Determine simulation object type which is returned by the cell_builder
    using FunctionReturnType = decltype(cell_builder({0, 0, 0}));

    auto container = rm->template Get<FunctionReturnType>();
    container->reserve(cells_per_dim[0] * cells_per_dim[1] * cells_per_dim[2]);
    for (size_t x = 0; x < cells_per_dim[0]; x++) {
      auto x_pos = x * space;
      for (size_t y = 0; y < cells_per_dim[1]; y++) {
        auto y_pos = y * space;
        for (size_t z = 0; z < cells_per_dim[2]; z++) {
          auto new_simulation_object = cell_builder({x_pos, y_pos, z * space});
          container->push_back(new_simulation_object);
        }
      }
    }
    container->Commit();
  }

  /// Adds simulation objects to the ResourceManager. Type of the simulation
  /// object is determined by the return type of parameter cell_builder.
  ///
  /// @param      positions     positions of the simulation objects to be
  /// @param      cell_builder  function containing the logic to instantiate a
  ///                           new simulation object. Takes `const
  ///                           std::array<double, 3>&` as input parameter
  ///
  template <typename Function, typename TResourceManager = ResourceManager<>>
  static void CreateCells(const std::vector<std::array<double, 3>>& positions,
                          Function cell_builder) {
    auto rm = TResourceManager::Get();
    // Determine simulation object type which is returned by the cell_builder
    using FunctionReturnType = decltype(cell_builder({0, 0, 0}));

    auto container = rm->template Get<FunctionReturnType>();
    container->reserve(positions.size());
    for (size_t i = 0; i < positions.size(); i++) {
      auto new_simulation_object =
          cell_builder({positions[i][0], positions[i][1], positions[i][2]});
      container->push_back(new_simulation_object);
    }
    container->Commit();
  }

  /// Adds simulation objects with random positions to the ResourceManager.
  /// Type of the simulation object is determined by the return type of
  /// parameter cell_builder.
  ///
  /// @param[in]  min           The minimum position value
  /// @param[in]  max           The maximum position value
  /// @param[in]  num_cells     The number cells
  /// @param[in]  cell_builder  function containing the logic to instantiate a
  ///                           new simulation object. Takes `const
  ///                           std::array<double, 3>&` as input parameter
  ///
  template <typename Function, typename TResourceManager = ResourceManager<>>
  static void CreateCellsRandom(double min, double max, int num_cells,
                                Function cell_builder) {
    auto rm = TResourceManager::Get();
    // Determine simulation object type which is returned by the cell_builder
    using FunctionReturnType = decltype(cell_builder({0, 0, 0}));

    auto container = rm->template Get<FunctionReturnType>();
    container->reserve(num_cells);

    // TODO(ahmad): throughout simulation only one random number generator
    // should be used, so this should go someplace accessible for other
    // classes / functions
    for (int i = 0; i < num_cells; i++) {
      double x = gTRandom.Uniform(min, max);
      double y = gTRandom.Uniform(min, max);
      double z = gTRandom.Uniform(min, max);
      auto new_simulation_object = cell_builder({x, y, z});
      container->push_back(new_simulation_object);
    }
    container->Commit();
  }

  /// Allows cells to secrete the specified substance. Diffusion throughout the
  /// simulation space is automatically taken care of by the DiffusionGrid class
  ///
  /// @param[in]  substance_id     The substance identifier
  /// @param[in]  substance_name   The substance name
  /// @param[in]  diffusion_coeff  The diffusion coefficient
  /// @param[in]  decay_constant   The decay constant
  /// @param[in]  resolution       The resolution of the diffusion grid
  ///
  template <typename TResourceManager = ResourceManager<>>
  static void DefineSubstance(int substance_id, std::string substance_name,
                              double diffusion_coeff, double decay_constant,
                              int resolution) {
    assert(resolution > 0 && "Resolution needs to be a positive integer value");
    auto rm = TResourceManager::Get();
    DiffusionGrid* d_grid =
        new DiffusionGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution);
    auto& diffusion_grids = rm->GetDiffusionGrids();
    diffusion_grids.push_back(d_grid);
  }

  template <typename TResourceManager = ResourceManager<>, typename F>
  static void InitializeSubstance(int substance_id, std::string substance_name,
                                  F function) {
    auto rm = TResourceManager::Get();
    auto diffusion_grid = rm->GetDiffusionGrid(substance_id);
    diffusion_grid->AddInitializer(function);
  }
};

}  // namespace bdm

#endif  // MODEL_INITIALIZER_H_
