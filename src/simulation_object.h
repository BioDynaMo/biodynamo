#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <type_traits>

#include "backend.h"
#include "type_util.h"

namespace bdm {

/// Contains code required by all simulation objects
template <typename TBackend = Scalar>
struct SimulationObject {
  using Backend = TBackend;

  template <typename T>
  friend struct SimulationObject;

  SimulationObject() : size_(1) {}

  virtual ~SimulationObject() {}

  /// Returns the number of elements in this object
  size_t size() const { return size_; }

  /// Equivalent to std::vector<> clear - it removes all elements from all
  /// data members
  void clear() { size_ = 0; }

  /// Appends a scalar element
  void push_back(const SimulationObject<Scalar>& other) { size_++; }

  /// Equivalent to std::vector<> reserve - it increases the capacity
  /// of all data member containers
  void reserve(size_t new_capacity) {}

  template <typename T = Backend>
  typename std::enable_if<std::is_same<T, SoaRef>::value,
                          SimulationObject<SoaRef>>::type
  operator=(const SimulationObject<Scalar>&) {
    return *this;
  }

 protected:
  /// Used internally to create the same object, but with
  /// different backend - required since inheritance chain is not known
  /// inside a mixin.
  template <typename TTBackend>
  using Self = SimulationObject<TTBackend>;

  template <typename T>
  SimulationObject(T* other, size_t idx) : idx_(idx), size_(other->size_) {}

  const size_t idx_ = 0;

 private:
  /// size_ is of type size_t& if TBackend == SoaRef; otherwise size_t
  typename type_ternary_operator<std::is_same<TBackend, SoaRef>::value, size_t&,
                                 size_t>::type size_;
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
