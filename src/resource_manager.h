#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <limits>
#include <tuple>
#include <utility>
#include "tuple_util.h"

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

template <typename... Types>
class ResourceManager {
 public:
  /// TypeBackend is Backend of the first type in Types
  using TypeBackend =
      typename std::tuple_element<0, std::tuple<Types...>>::type::Backend;
  template <typename T>
  /// Determine Container based on the Backend
  using TypeContainer = typename TypeBackend::template Container<T>;

  /// Singleton pattern - return the only instance with this template parameters
  static ResourceManager<Types...>* Get() {
    static ResourceManager<Types...> kInstance;
    return &kInstance;
  }

  /// Return the container of this Type
  template <typename Type>
  TypeContainer<Type>* Get() {
    return &std::get<TypeContainer<Type>>(data_);
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
  std::tuple<TypeContainer<Types>...> data_;
};

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
