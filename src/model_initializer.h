#ifndef MODEL_INITIALIZER_H_
#define MODEL_INITIALIZER_H_

#include "resource_manager.h"

namespace bdm {

struct ModelInitializer {
  /// Creates a 3D grid of simulation objects and adds them to the
  /// ResourceManager. Type of the simulation object is determined by the return
  /// type of parameter function.
  ///
  ///      ModelInitializer::Grid3D(8, 10, [](const std::array<double, 3>& pos){
  ///        return Cell(pos);
  ///      });
  /// @param cells_per_dim number of simulation objects on each axis.
  ///        Number of generated simulation objects = `cells_per_dim ^ 3`
  /// @param space space between the positions -
  ///        e.g space = 10: positions =
  ///        `{(0, 0, 0), (0, 0, 10), (0, 0, 20), ... }`
  /// @param function function containing the logic to instantiate a new
  ///         simulation object. Takes `const std::array<double, 3>&` as input
  ///         parameter
  template <typename Function, typename TResourceManager = ResourceManager<>>
  static void Grid3D(size_t cells_per_dim, double space, Function function) {
    auto rm = TResourceManager::Get();
    // determine simulation object type which is returned by the function
    using FunctionReturnType = decltype(function({0, 0, 0}));

    auto container = rm->template Get<FunctionReturnType>();
    container->reserve(cells_per_dim * cells_per_dim * cells_per_dim);
    for (size_t x = 0; x < cells_per_dim; x++) {
      auto x_pos = x * space;
      for (size_t y = 0; y < cells_per_dim; y++) {
        auto y_pos = y * space;
        for (size_t z = 0; z < cells_per_dim; z++) {
          auto new_simulation_object = function({x_pos, y_pos, z * space});
          container->push_back(new_simulation_object);
        }
      }
    }
    container->Commit();
  }
};

}  // namespace bdm

#endif  // MODEL_INITIALIZER_H_
