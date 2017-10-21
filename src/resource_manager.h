#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <Rtypes.h>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "backend.h"
#include "diffusion_grid.h"
#include "tuple_util.h"
#include "variadic_template_parameter_util.h"

namespace bdm {

/// Unique identifier of a simulation object. Acts as a type erased pointer.
/// Has the same type for every simulation object.
/// The id is split into two parts: Type index and element index.
/// The first one is used to obtain the container in the ResourceManager, the
/// second specifies the element within this vector.
class SoHandle {
 public:
  SoHandle() noexcept
      : type_idx_(std::numeric_limits<decltype(type_idx_)>::max()),
        element_idx_(std::numeric_limits<decltype(element_idx_)>::max()) {}
  SoHandle(uint16_t type_idx, uint32_t element_idx)
      : type_idx_(type_idx), element_idx_(element_idx) {}
  uint16_t GetTypeIdx() const { return type_idx_; }
  uint32_t GetElementIdx() const { return element_idx_; }

  bool operator==(const SoHandle& other) const {
    return type_idx_ == other.type_idx_ && element_idx_ == other.element_idx_;
  }

  bool operator!=(const SoHandle& other) const { return !(*this == other); }

  bool operator<(const SoHandle& other) const {
    if (type_idx_ == other.type_idx_) {
      return element_idx_ < other.element_idx_;
    } else {
      return type_idx_ < other.type_idx_;
    }
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const SoHandle& handle) {
    stream << "Type idx: " << handle.type_idx_
           << " element idx: " << handle.element_idx_;
    return stream;
  }

 private:
  uint16_t type_idx_;
  /// changed element index to uint32_t after issues with std::atomic with
  /// size 16 -> max element_idx: 4.294.967.296
  uint32_t element_idx_;
};

namespace detail {

/// \see bdm::ConvertToContainerTuple, VariadicTypedef
template <typename Backend, typename... Types>
struct ConvertToContainerTuple {};

/// \see bdm::ConvertToContainerTuple, VariadicTypedef
template <typename Backend, typename... Types>
struct ConvertToContainerTuple<Backend, VariadicTypedef<Types...>> {
  // Helper alias to get the container type associated with Backend
  template <typename T>
  using Container = typename Backend::template Container<T>;
  // Helper type alias to get a type with certain Backend
  template <typename T>
  using ToBackend = typename T::template Self<Backend>;
  using type = std::tuple<Container<ToBackend<Types>>...>;  // NOLINT
};

}  // namespace detail

/// Create a tuple of types in the parameter pack and wrap each type with
/// container.
/// @tparam Backend in which the variadic types should be stored in
/// @tparam TVariadicTypedefWrapper type that wraps a VariadicTypedef
/// which in turn contains the variadic template parameters
/// \see VariadicTypedefWrapper
template <typename Backend, typename TVariadicTypedef>
struct ConvertToContainerTuple {
  typedef
      typename detail::ConvertToContainerTuple<Backend, TVariadicTypedef>::type
          type;  // NOLINT
};

/// Forward declaration for concrete compile time parameter.
/// Will be used as default template parameter.
template <typename TBackend = Soa>
struct CompileTimeParam;

/// ResourceManager holds a container for each atomic type in the simulation.
/// It provides methods to get a certain container, execute a function on a
/// a certain element, all elements of a certain type or all elements inside
/// the ResourceManager. Elements are uniquely identified with its SoHandle.
/// Furthermore, the types specified in AtomicTypes are backend invariant
/// Hence it doesn't matter which version of the Backend is specified.
/// ResourceManager internally uses the TBackendWrapper parameter to convert
/// all atomic types to the desired backend.
/// This makes user code easier since atomic types can be specified as scalars.
/// @tparam TCompileTimeParam type that containes the compile time parameter for
/// a specific simulation. ResourceManager extracts Backend and AtomicTypes.
template <typename TCompileTimeParam = CompileTimeParam<>>
class ResourceManager {
 public:
  using Backend = typename TCompileTimeParam::SimulationBackend;
  using Types = typename TCompileTimeParam::AtomicTypes;
  /// Determine Container based on the Backend
  template <typename T>
  using TypeContainer = typename Backend::template Container<T>;
  /// Helper type alias to get a type with certain Backend
  template <typename T>
  using ToBackend = typename T::template Self<Backend>;

  /// Singleton pattern - return the only instance with this template parameters
  static ResourceManager<TCompileTimeParam>* Get() { return instance_.get(); }

  /// Return the container of this Type
  /// @tparam Type atomic type whose container should be returned
  ///         invariant to the Backend. This means that even if ResourceManager
  ///         stores e.g. `SoaCell`, Type can be `Cell` and still returns the
  ///         correct container.
  template <typename Type>
  TypeContainer<ToBackend<Type>>* Get() {
    return &std::get<TypeContainer<ToBackend<Type>>>(data_);
  }

  /// Return the container of diffusion grids
  std::vector<DiffusionGrid*>& GetDiffusionGrids() { return diffusion_grids_; }

  /// Return the diffusion grid which holds the substance of specified name
  DiffusionGrid* GetDiffusionGrid(size_t substance_id) {
    assert(substance_id < diffusion_grids_.size() &&
           "You tried to access a diffusion grid that does not exist!");
    return diffusion_grids_[substance_id];
  }

  /// Returns the total number of simulation objects
  size_t GetNumSimObjects() {
    size_t num_so = 0;
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i,
                   [&](auto* container) { num_so += container->size(); });
    }
    return num_so;
  }

  /// Default constructor. Unfortunately needs to be public although it is
  /// a singleton to be able to use ROOT I/O
  ResourceManager() {
    // Soa container contain one element upon construction
    Clear();
  }

  /// Free the memory that was reserved for the diffusion grids
  virtual ~ResourceManager() {
    for (auto grid : diffusion_grids_) {
      delete grid;
    }
  }

  /// Apply a function on a certain element
  /// @param handle - simulation object id; specifies the tuple index and
  /// element index \see SoHandle
  /// @param function that will be called with the element as a parameter
  ///
  ///     rm->ApplyOnElement(handle, [](auto& element) {
  ///                          std::cout << element << std::endl;
  ///                       });
  template <typename TFunction>
  void ApplyOnElement(SoHandle handle, TFunction&& function) {
    auto type_idx = handle.GetTypeIdx();
    auto element_idx = handle.GetElementIdx();
    ::bdm::Apply(&data_, type_idx,
                 [&](auto* container) { function((*container)[element_idx]); });
  }

  /// Apply a function on all container types
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, uint16_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  template <typename TFunction>
  void ApplyOnAllTypes(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&](auto* container) { function(container, i); });
    }
  }

  /// Apply a function on all elements in every container
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllElements([](auto& element, SoHandle handle) {
  ///                              std::cout << element << std::endl;
  ///                          });
  template <typename TFunction>
  void ApplyOnAllElements(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&](auto* container) {
        for (size_t e = 0; e < container->size(); e++) {
          function((*container)[e], SoHandle(i, e));
        }
      });
    }
  }

  /// Apply a function on all elements in every container
  /// Function invocations are parallelized
  /// \see ApplyOnAllElements
  template <typename TFunction>
  void ApplyOnAllElementsParallel(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&](auto* container) {
#pragma omp parallel for
        for (size_t e = 0; e < container->size(); e++) {
          function((*container)[e], SoHandle(i, e));
        }
      });
    }
  }

  /// Remove elements from each type
  void Clear() {
    ApplyOnAllTypes(
        [](auto* container, uint16_t type_idx) { container->clear(); });
  }

  template <typename TSo>
  void push_back(const TSo& so) {  // NOLINT
    Get<TSo>()->push_back(so);
  }

  /// Returns the number of simulation object types
  static constexpr size_t NumberOfTypes() {
    return std::tuple_size<decltype(data_)>::value;
  }

 private:
  static std::unique_ptr<ResourceManager<TCompileTimeParam>> instance_;

  /// creates one container for each type in Types.
  /// Container type is determined based on the specified Backend
  typename ConvertToContainerTuple<Backend, Types>::type data_;
  std::vector<DiffusionGrid*> diffusion_grids_;

  friend class SimulationBackup;
  ClassDefNV(ResourceManager, 1);
};

template <typename T>
std::unique_ptr<ResourceManager<T>> ResourceManager<T>::instance_ =
    std::unique_ptr<ResourceManager<T>>(new ResourceManager<T>());

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
