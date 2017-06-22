#ifndef BIOLOGY_MODULE_UTIL_H_
#define BIOLOGY_MODULE_UTIL_H_

#include "mpark/variant.hpp"

namespace bdm {

using mpark::variant;
using mpark::visit;

/// Events used in biology modules to decide whether it should be copied
enum Event { kCellDivision, kNeuriteBranching };

/// \brief Used for simulation objects where biology modules are not used.
/// variant implementation does not allow `variant<>`
/// -> `variant<NullBiologyModule>`
struct NullBiologyModule {
  template <typename T>
  void Run(T* t) {}

  bool IsCopied(Event event) const { return false; }
};

/// \brief Visitor to execute the `Run` method of a biology module
/// @tparam TSimulationObject type of simulation object that owns the biology
///         module
template <typename TSimulationObject>
struct RunVisitor {
  /// @param so pointer to the simulation object on which the biology module
  ///        should be executed
  explicit RunVisitor(TSimulationObject* const so) : kSimulationObject(so) {}

  template <typename T>
  void operator()(T& t) const {
    t.Run(kSimulationObject);
  }

 private:
  TSimulationObject* const kSimulationObject;
};

/// \brief Visitor to copy biology modules from one structure to another
/// @tparam TVector type of the destination container where the biology modules
///         are stored
template <typename TVector>
struct CopyVisitor {
  /// @param event that lead to the copy operation - e.g. cell division:
  ///        biology modules should be copied from mother to daughter cell
  /// @param vector biology module vector in which the copied module will be
  ///        inserted
  CopyVisitor(Event event, TVector* vector) : kEvent(event), vector_(vector) {}

  template <typename T>
  void operator()(const T& from) const {
    if (from.IsCopied(kEvent)) {
      T copy(from);  // NOLINT
      vector_->emplace_back(std::move(copy));
    }
  }

  const Event kEvent;
  TVector* vector_;
};

/// Forward declaration which is used as default template
/// parameter. Must be defined in the concreate simulation.
struct BiologyModules;

/// Preprocessor macro to make definition of BiologyModules easier (simplified
/// syntax) \n
/// CAUTION: Needs to be called inside namespace `::bdm`, since `BiologyModules`
/// was forward declared in this namespace. Otherwise compilation will fail.
#define BDM_DEFINE_BIOLOGY_MODULES(...)                 \
  struct BiologyModules : public variant<__VA_ARGS__> { \
    using variant::variant;                             \
  };

/// Simplified call for `BDM_DEFINE_BIOLOGY_MODULES` for empty BiologyModules
#define BDM_DEFAULT_BIOLOGY_MODULES() \
  BDM_DEFINE_BIOLOGY_MODULES(NullBiologyModule)

}  // namespace bdm

#endif  // BIOLOGY_MODULE_UTIL_H_
