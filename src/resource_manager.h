#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <limits>
#include <tuple>
#include <utility>
#include "tuple_util.h"
#include "variadic_template_parameter_util.h"

namespace bdm {

/// Specifies the number of bits that are used in a SoHandle to adress the
/// type. Maximum number of types in the ResourceManager is `2^kTypeIdBits`
/// e.g.: kTypeIdBits = 4; maximum number of Types = 16
static constexpr uint8_t kTypeIdBits = 4;

/// Unique identifier of a simulation object. Acts as a type erased pointer.
/// Has the same type for every simulation object.
/// The id is split into two parts: Type index and element index.
/// The first one is used to obtain the container in the ResourceManager, the
/// second specifies the element within this vector.
/// The number of bits used for the type index is specified in `kTypeIdBits`.
///
///      |___________|____________________________________________________|
///     63  type_idx                  element_idx                         0
using SoHandle = uint64_t;

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
  typedef std::tuple<Container<ToBackend<Types>>...> type;  // NOLINT
};

}  // namespace detail

/// Create a tuple of types in the parameter pack and wrap each type witch
/// container
/// @tparam Backend in which the variadic types should be stored in
/// @tparam TVariadicTypedefWrapper type that wraps a VariadicTypedef
/// which in turn contains the variadic template parameters
/// \see VariadicTypedefWrapper
template <typename Backend, typename TVariadicTypedefWrapper>
struct ConvertToContainerTuple {
  typedef typename detail::ConvertToContainerTuple<
      Backend, typename TVariadicTypedefWrapper::types>::type type;  // NOLINT
};

/// Forward declaration of atomic types used in the simulation.
/// Must be specified in the simulation code using the macro
/// \see BDM_DEFINE_ATOMIC_TYPES
struct AtomicTypes;

/// This forward declaration wraps the backend in which the atomic types
/// should be stored in. \see BDM_DEFINE_BACKEND
struct BackendWrapper;

/// Forward declaration of default backend requires wrapping Backend.
/// This method makes testing easier since the wrapper does not have to be
/// defined manually
template <typename Backend>
struct InlineBackendWrapper {
  typedef Backend type;  // NOLINT
};

/// ResourceManager holds a container for each atomic type in the simulation.
/// It provides methods to get a certain container, execute a function on a
/// a certain element, all elements of a certain type or all elements inside
/// the ResourceManager. Elements are uniquely identified with its SoHandle.
/// Furthermore, the types specified in AtomicTypes are backend invariant
/// Hence it doesn't matter which version of the Backend is specified.
/// ResourceManager internally uses the TBackendWrapper parameter to convert
/// all atomic types to the desired backend.
/// This makes user code easier since atomic types can be specified as scalars.
/// @tparam TBackendWrapper is a wrapper around Backend \see BDM_DEFINE_BACKEND
/// @tparam Types is a wrapper that contains a VariadicTypedef. Hence, it is
/// possible to pass a variable number of types into it. \see VariadicTypedef
/// BDM_DEFINE_ATOMIC_TYPES
template <typename TBackendWrapper = BackendWrapper,
          typename Types = AtomicTypes>
class ResourceManager {
 public:
  using Backend = typename TBackendWrapper::type;
  /// Determine Container based on the Backend
  template <typename T>
  using TypeContainer = typename Backend::template Container<T>;
  /// Helper type alias to get a type with certain Backend
  template <typename T>
  using ToBackend = typename T::template Self<Backend>;

  /// Singleton pattern - return the only instance with this template parameters
  static ResourceManager<TBackendWrapper, Types>* Get() {
    static ResourceManager<TBackendWrapper, Types> kInstance;
    return &kInstance;
  }

  /// Return the container of this Type
  /// @tparam Type atomic type whose container should be returned
  ///         invariant to the Backend. This means that even if ResourceManager
  ///         stores e.g. `SoaCell`, Type can be `Cell` and still returns the
  ///         correct container.
  template <typename Type>
  TypeContainer<ToBackend<Type>>* Get() {
    return &std::get<ToBackend<ToBackend<Type>>>(data_);
  }

  /// Apply a function on a certain element
  /// @param handle - simulation object id; specifies the tuple index and
  /// element
  ///        index \see SoHandle
  /// @param function that will be called with the element as a parameter
  ///
  ///     rm->ApplyOnElement(handle, [](auto& element) {
  ///                          std::cout << element << std::endl;
  ///                       });
  template <typename TFunction>
  void ApplyOnElement(SoHandle handle, TFunction&& function) {
    // auto type_idx = index >> 63;
    auto type_idx = GetTypeIdx(handle);
    auto element_idx = GetElementIdx(handle);
    ::bdm::Apply(&data_, type_idx, [&element_idx, &function](auto& element) {
      function(element[element_idx]);
    });
  }

  /// Apply a function on all container types
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes(handle, [](auto& container) {
  ///                          std::cout << container.size() << std::endl;
  ///                       });
  template <typename TFunction>
  void ApplyOnAllTypes(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (size_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, function);
    }
  }

  /// Apply a function on all elements in every container
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllElements(handle, [](auto& element) {
  ///                              std::cout << element << std::endl;
  ///                          });
  template <typename TFunction>
  void ApplyOnAllElements(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (size_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&function](auto& container) {
        for (size_t e = 0; e < container.size(); e++) {
          function(container[e]);
        }
      });
    }
  }

  /// Remove elements from each type
  void Clear() {
    ApplyOnAllTypes([](auto& container) { container.clear(); });
  }

  /// Generate a SoHandle
  /// @param element_idx index of the element within the container
  /// @param type_idx index of the container within the std::tuple
  static SoHandle GenSoHandle(size_t element_idx, size_t type_idx) {
    using Limit = std::numeric_limits<size_t>;
    size_t mask = type_idx << (Limit::digits - kTypeIdBits);
    return element_idx |= mask;
  }

 private:
  /// Return type index based on SoHandle
  /// \see SoHandle
  static size_t GetTypeIdx(SoHandle handle) {
    using Limit = std::numeric_limits<size_t>;
    return handle >> (Limit::digits - kTypeIdBits);
  }

  /// Return element index based on SoHandle
  /// \see SoHandle
  static size_t GetElementIdx(SoHandle handle) {
    using Limit = std::numeric_limits<size_t>;
    const size_t mask = Limit::max() >> kTypeIdBits;
    return handle & mask;
  }

  /// creates one container for each type in Types.
  /// Container type is determined based on the backend of the types
  typename ConvertToContainerTuple<Backend, Types>::type data_;
};

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
