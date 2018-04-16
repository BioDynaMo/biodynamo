#ifndef SIMULATION_OBJECT_VECTOR_H_
#define SIMULATION_OBJECT_VECTOR_H_

#include <vector>
#include "resource_manager.h"

namespace bdm {

/// Two dimensional vector. Creates one vector for each simulation type
/// Use this vector if you need one object of type T for each simulation object
template <typename T, typename TResourceManager = ResourceManager<>>
class SimulationObjectVector {
 public:
  SimulationObjectVector() {
    data_.resize(TResourceManager::NumberOfTypes());
    Initialize();
  }

  /// resize vectors to match number of simulation object of the corresponding
  /// type.
  /// e.g. ResourceManager has two types `A` and `B`. `A` has 10 elements
  /// and `B` 20. `data_[0]` corresponds to `A` and is therefore resized to
  /// 10 elements, while `data_[1]` corresponds to `B` and is resized to 20
  /// elements.
  void Initialize() {
    clear();
    auto rm = TResourceManager::Get();
    rm->ApplyOnAllTypes([&](auto* sim_objects, uint16_t type_idx) {
      data_[type_idx].resize(sim_objects->size());
    });
  }

  void clear() {  // NOLINT
    for (auto& vec : data_) {
      vec.clear();
    }
  }

  // Returns the number of element types
  size_t size() {
    return data_.size();
  }

  // Returns the number of elements of specified type
  size_t size(uint16_t type_idx) {
    return data_[type_idx].size();
  }

  const T& operator[](const SoHandle& handle) const {
    return data_[handle.GetTypeIdx()][handle.GetElementIdx()];
  }

  T& operator[](const SoHandle& handle) {
    return data_[handle.GetTypeIdx()][handle.GetElementIdx()];
  }

 private:
  /// one std::vector<T> for each type in ResourceManager
  std::vector<std::vector<T>> data_;
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_VECTOR_H_
