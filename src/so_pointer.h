// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef SO_POINTER_H_
#define SO_POINTER_H_

#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

#include "backend.h"
#include "simulation.h"
#include "simulation_backup.h"

namespace bdm {

/// Simulation object pointer. Required to point into simulation objects with
/// `Soa` backend. `SoaRef` has the drawback that its size depends on the number
/// of data members. Benefit compared to SoHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations
/// @tparam TSoSimBackend simulation object type with simulation backend
/// @tparam TBackend backend - required to avoid extracting it from
///         TSoSimBackend which would result in "incomplete type errors" in
///         certain cases.
/// NB: ROOT IO only supports `so_container_` that point into the
/// `ResourceManager`. Separate containers will not be serialized correctly!
template <typename TSo>
class SoPointer {
  /// Determine correct container
  using Container = TransactionalVector<TSo>;

 public:
  SoPointer(Container* container, uint64_t element_idx)
      : so_container_(container), element_idx_(element_idx) {}

  /// constructs an SoPointer object representing a nullptr
  SoPointer() {}

  uint32_t GetElementIdx() const { return element_idx_; }
  void SetElementIdx(uint32_t element_idx) { element_idx_ = element_idx; }

  /// Equals operator that enables the following statement `so_ptr == nullptr;`
  bool operator==(std::nullptr_t) const {
    return element_idx_ == std::numeric_limits<uint64_t>::max();
  }

  /// Not equal operator that enables the following statement `so_ptr !=
  /// nullptr;`
  bool operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

  bool operator==(const SoPointer& other) const {
    return element_idx_ == other.element_idx_ &&
           so_container_ == other.so_container_;
  }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `so_ptr = nullptr;`
  SoPointer& operator=(std::nullptr_t) {
    element_idx_ = std::numeric_limits<uint64_t>::max();
    return *this;
  }

  TSo& operator->() {
    assert(*this != nullptr);
    return (*so_container_)[element_idx_];
  }

  const TSo& operator->() const {
    assert(*this != nullptr);
    return (*so_container_)[element_idx_];
  }

  friend std::ostream& operator<<(std::ostream& str, const SoPointer<TSo>& so_ptr) {
    str << "{ container: " << so_ptr.so_container_
        << ", element_idx: " << so_ptr.element_idx_ << "}";
    return str;
  }

 private:
  Container* so_container_ = nullptr;
  uint64_t element_idx_ = std::numeric_limits<uint64_t>::max();

  ClassDef(SoPointer, 1);
};

// namespace detail {
//
// /// Enum for the three possible states of `SoPointer::so_container_`
// enum ContainerPointerState {
//   /// `SoPointer::so_container_` points inside the `ResourceManager`
//   kPointIntoRm,
//   /// `SoPointer::so_container_` is a nullptr
//   kNullPtr,
//   /// `SoPointer::so_container_` points into a separate container
//   kSeparate
// };
//
// /// Functor to read `SoPointer::so_container_` from a ROOT file.
// /// Uses dynamic dispatch to avoid compilation errors between incompatible
// /// types. TODO(lukas) Once we use C++17 use if constexpr instead to simplify
// /// the logic.
// template <typename TSoSimBackend, typename TBackend>
// struct ReadContainerFunctor {
//   /// Backends between SoPointer and ResourceManager are matching
//   template <typename TContainer, typename TTBackend = TBackend,
//             typename TSimulation = Simulation<>>
//   typename std::enable_if<std::is_same<
//       TTBackend, typename TSimulation::ResourceManager_t::Backend>::value>::type
//   operator()(TBuffer& R__b, TContainer** container, uint64_t) {  // NOLINT
//     int state;
//     R__b >> state;
//     if (state == ContainerPointerState::kPointIntoRm) {
//       R__b >> *container;
//       // if a whole simulation is restored from a ROOT file, `TSimulation::Get`
//       // is
//       // not updated to the new `ResourceManager` yet. Therefore, we must delay
//       // this call. It will be executed after the restore operation has been
//       // completed.
//       SimulationBackup::after_restore_event_.push_back([=]() {
//         auto* rm = TSimulation::GetActive()->GetResourceManager();
//         *container = rm->template Get<TSoSimBackend>();
//       });
//     } else if (state == ContainerPointerState::kSeparate) {
//       R__b >> *container;
//     } else {
//       R__b >> *container;
//     }
//   }
//
//   /// Backends not matching, `SoPointer::so_container_` is certainly not
//   /// pointing into `ResourceManager<>`
//   template <typename TContainer>
//   void operator()(TBuffer& R__b, TContainer** container, ...) {  // NOLINT
//     int state;
//     R__b >> state;
//     if (state == ContainerPointerState::kSeparate) {
//       R__b >> *container;
//     } else {
//       // Read nullptr
//       R__b >> *container;
//     }
//   }
// };
//
// /// Functor to write `SoPointer::so_container_` from a ROOT file.
// /// Uses dynamic dispatch to avoid compilation errors between incompatible
// /// types. TODO(lukas) Once we use C++17 use if constexpr instead to simplify
// /// the logic.
// template <typename TSoSimBackend, typename TBackend>
// struct WriteContainerFunctor {
//   /// Backends between SoPointer and ResourceManager are matching
//   template <typename TContainer, typename TTBackend = TBackend,
//             typename TSimulation = Simulation<>>
//   typename std::enable_if<std::is_same<
//       TTBackend, typename TSimulation::ResourceManager_t::Backend>::value>::type
//   operator()(TBuffer& R__b, const TContainer* container, uint64_t) {  // NOLINT
//     auto* rm = TSimulation::GetActive()->GetResourceManager();
//     if (container == nullptr) {
//       // write nullptr
//       R__b << ContainerPointerState::kNullPtr;
//       R__b << container;
//     } else if (rm->template Get<TSoSimBackend>() == container) {
//       R__b << ContainerPointerState::kPointIntoRm;
//       // skip container inside ResourceManager and write nullptr instead
//       // ROOT does not recognise that the container points inside the
//       // ResourcManager and would duplicate the container.
//       TContainer* n = nullptr;
//       R__b << n;
//     } else {
//       // write separate container
//       R__b << ContainerPointerState::kSeparate;
//       R__b << container;
//     }
//   }
//
//   /// Backends not matching, `SoPointer::so_container_ is certainly not pointing
//   /// into `ResourceManager<>`
//   template <typename TContainer>
//   void operator()(TBuffer& R__b, const TContainer* container, ...) {  // NOLINT
//     if (container != nullptr) {
//       R__b << ContainerPointerState::kSeparate;
//       R__b << container;
//     } else {
//       // write nullptr
//       R__b << ContainerPointerState::kNullPtr;
//       R__b << container;
//     }
//   }
// };
//
// }  // namespace detail
//
// /// Custom streamer for `bdm::SoPointer`.
// /// This is necessary because ROOT does not detect if
// /// `so_container_` points inside `ResourceManager`. Therefore,
// /// ROOT by default duplicates the container and therefore breaks the link.
// /// Thus, a custom streamer is required.
// ///    It detects if `so_container_` points into `ResourceManager`. If so it
// ///    does not persist `so_container_`. Otherwise, if it is a separate
// ///    container it will be persisted. The third option is that `so_container_`
// ///    is a `nullptr` which is treated differently during the read back stage.
// template <typename TSoSimBackend, typename TBackend>
// inline void SoPointer<TSoSimBackend, TBackend>::Streamer(
//     TBuffer& R__b) {  // NOLINT
//   if (R__b.IsReading()) {
//     R__b >> element_idx_;
//     detail::ReadContainerFunctor<TSoSimBackend, TBackend> restore_container;
//     restore_container(R__b, &so_container_, 0);
//   } else {
//     R__b << element_idx_;
//     detail::WriteContainerFunctor<TSoSimBackend, TBackend> write_container;
//     write_container(R__b, so_container_, 0);
//   }
// }

}  // namespace bdm

#endif  // SO_POINTER_H_
