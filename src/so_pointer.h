#ifndef SO_POINTER_H_
#define SO_POINTER_H_

#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

#include "backend.h"
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
template <typename TSoSimBackend, typename TBackend>
class SoPointer {
  /// Determine correct container
  using Container = typename TBackend::template Container<TSoSimBackend>;

 public:
  SoPointer(Container* container, uint64_t element_idx)
      : so_container_(container), element_idx_(element_idx) {}

  /// constructs an SoPointer object representing a nullptr
  SoPointer() {}

  uint32_t GetElementIdx() const { return element_idx_; }
  void SetElementIdx(uint32_t element_idx) { element_idx_ = element_idx; }

  /// TODO change to operator `so_ptr == nullptr` or `so_ptr != nullptr`
  bool IsNullPtr() const {
    return element_idx_ == std::numeric_limits<uint64_t>::max();
  }

  /// Equals operator that enables the following statement `so_ptr == nullptr;`
  bool operator==(std::nullptr_t) const {
    return element_idx_ == std::numeric_limits<uint64_t>::max();
  }

  /// Not equal operator that enables the following statement `so_ptr !=
  /// nullptr;`
  bool operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

  bool operator==(const SoPointer<TSoSimBackend, TBackend>& other) const {
    return element_idx_ == other.element_idx_ &&
           so_container_ == other.so_container_;
  }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `so_ptr = nullptr;`
  SoPointer<TSoSimBackend, TBackend>& operator=(std::nullptr_t) {
    element_idx_ = std::numeric_limits<uint64_t>::max();
    return *this;
  }

  /// Method to return the object it points to. Unfortunately, it is not
  /// possible to use `operator->`, which would lead to a nice syntax like:
  /// `so_ptr->SomeFunction()`. `operator->` must return a pointer which is
  /// not possible for Soa backends (`operator[]` returns a temporary SoaRef
  /// object).
  template <typename TTBackend = TBackend>
  auto& Get(
      typename std::enable_if<std::is_same<TTBackend, Scalar>::value>::type* p =
          0) {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
    // return (*rm->Get<TSoSimBackend>())[element_idx_];
  }

  template <typename TTBackend = TBackend>
  const auto& Get(
      typename std::enable_if<std::is_same<TTBackend, Scalar>::value>::type* p =
          0) const {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
  }

  template <typename TTBackend = TBackend>
  auto Get(typename std::enable_if<std::is_same<TTBackend, Soa>::value>::type*
               p = 0) {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
  }

  template <typename TTBackend = TBackend>
  const auto Get(
      typename std::enable_if<std::is_same<TTBackend, Soa>::value>::type* p =
          0) const {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
  }

  friend std::ostream& operator<<(
      std::ostream& str, const SoPointer<TSoSimBackend, TBackend>& so_ptr) {
    str << "{ container: " << so_ptr.so_container_
        << ", element_idx: " << so_ptr.element_idx_ << "}";
    return str;
  }

 private:
  Container* so_container_ = nullptr;
  uint64_t element_idx_ = std::numeric_limits<uint64_t>::max();

  BDM_TEMPLATE_CLASS_DEF_CUSTOM_STREAMER(SoPointer, 1);
};

namespace detail {

/// Enum for the three possible states of `SoPointer::so_container_`
enum ContainerPointerState {
  /// `SoPointer::so_container_` points inside the `ResourceManager`
  kPointIntoRm,
  /// `SoPointer::so_container_` is a nullptr
  kNullPtr,
  /// `SoPointer::so_container_` points into a separate container
  kSeparate
};

/// Functor to read `SoPointer::so_container_` from a ROOT file.
/// Uses dynamic dispatch to avoid compilation errors between incompatible
/// types. TODO(lukas) Once we use C++17 use if constexpr instead to simplify
/// the logic.
template <typename TSoSimBackend, typename TBackend>
struct ReadContainerFunctor {
  /// Backends between SoPointer and ResourceManager are matching
  template <typename TContainer, typename TTBackend = TBackend,
            typename TRm = ResourceManager<>>
  typename std::enable_if<
      std::is_same<TTBackend, typename TRm::Backend>::value>::type
  operator()(TBuffer& R__b, TContainer** container, uint64_t) {
    int state;
    R__b >> state;
    if (state == ContainerPointerState::kPointIntoRm) {
      R__b >> *container;
      // if a whole simulation is restored from a ROOT file, `TRm::Get` is not
      // updated to the new `ResourceManager` yet. Therefore, we must delay this
      // call. It will be executed after the restore operation has been
      // completed.
      SimulationBackup::after_restore_event_.push_back([&container]() {
        *container = TRm::Get()->template Get<TSoSimBackend>();
      });
    } else if (state == ContainerPointerState::kSeparate) {
      R__b >> *container;
    } else {
      R__b >> *container;
    }
  }

  /// Backends not matching, `SoPointer<..>::container is certainly not pointing
  /// into `ResourceManager<>`
  template <typename TContainer>
  void operator()(TBuffer& R__b, TContainer** container, ...) {
    int state;
    R__b >> state;
    if (state == ContainerPointerState::kSeparate) {
      R__b >> *container;
    } else {
      // Read nullptr
      R__b >> *container;
    }
  }
};

/// Functor to write `SoPointer::so_container_` from a ROOT file.
/// Uses dynamic dispatch to avoid compilation errors between incompatible
/// types. TODO(lukas) Once we use C++17 use if constexpr instead to simplify
/// the logic.
template <typename TSoSimBackend, typename TBackend>
struct WriteContainerFunctor {
  /// Backends between SoPointer and ResourceManager are matching
  template <typename TContainer, typename TTBackend = TBackend,
            typename TRm = ResourceManager<>>
  typename std::enable_if<
      std::is_same<TTBackend, typename TRm::Backend>::value>::type
  operator()(TBuffer& R__b, const TContainer* container, uint64_t) {
    if (container == nullptr) {
      // write nullptr
      R__b << ContainerPointerState::kNullPtr;
      R__b << container;
    } else if (TRm::Get()->Get()->template Get<TSoSimBackend>() == container) {
      R__b << ContainerPointerState::kPointIntoRm;
      // skip container inside ResourceManager and write nullptr instead
      // ROOT does not recognise that the container points inside the
      // ResourcManager and would duplicate the container.
      TContainer* n = nullptr;
      R__b << n;
    } else {
      // write separate container
      R__b << ContainerPointerState::kSeparate;
      R__b << container;
    }
  }

  /// Backends not matching, `SoPointer<..>::container is certainly not pointing
  /// into `ResourceManager<>`
  template <typename TContainer>
  void operator()(TBuffer& R__b, const TContainer* container, ...) {
    if (container != nullptr) {
      R__b << ContainerPointerState::kSeparate;
      R__b << container;
    } else {
      // write nullptr
      R__b << ContainerPointerState::kNullPtr;
      R__b << container;
    }
  }
};

}  // namespace detail

/// Custom streamer for `bdm::SoPointer`.
/// This is necessary because ROOT does not detect if
/// `so_container_` points inside `ResourceManager`. Therefore,
/// ROOT by default duplicates the container and therefore breaks the link.
/// Thus, a custom streamer is required.
///    It detects if `so_container_` points into `ResourceManager`. If so it
///    does not persist `so_container_`. Otherwise, if it is a separate
///    container it will be persisted. The third option is that `so_container_`
///    is a `nullptr` which is treated differently during the read back stage.
template <typename TSoSimBackend, typename TBackend>
inline void SoPointer<TSoSimBackend, TBackend>::Streamer(
    TBuffer& R__b) {  // NOLINT
  if (R__b.IsReading()) {
    R__b >> element_idx_;
    detail::ReadContainerFunctor<TSoSimBackend, TBackend> restore_container;
    restore_container(R__b, &so_container_, 0);
  } else {
    R__b << element_idx_;
    detail::WriteContainerFunctor<TSoSimBackend, TBackend> write_container;
    write_container(R__b, so_container_, 0);
  }
}

}  // namespace bdm

#endif  // SO_POINTER_H_
