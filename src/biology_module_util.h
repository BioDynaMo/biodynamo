#ifndef BIOLOGY_MODULE_UTIL_H_
#define BIOLOGY_MODULE_UTIL_H_

#include "variant.h"

namespace bdm {

/// Events used in biology modules to decide whether it should be copied
enum Event { kCellDivision, kNeuriteBranching };

/// \brief Used for simulation objects where biology modules are not used.
/// variant implementation does not allow `variant<>`
/// -> `variant<NullBiologyModule>`
struct NullBiologyModule {
  template <typename T>
  void Run(T* t) {}

  bool IsCopied(Event event) const { return false; }
  ClassDefNV(NullBiologyModule, 1);
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

}  // namespace bdm

#endif  // BIOLOGY_MODULE_UTIL_H_
