#ifndef TRANSACTIONAL_VECTOR_H_
#define TRANSACTIONAL_VECTOR_H_

#include <algorithm>
#include <cassert>
#include <exception>
#include <mutex>
#include <vector>

#include "linkdef_util.h"
#include "root_util.h"

namespace bdm {

/// TransactionalVector has methods to add and remove elements which must be
/// commited before the changes take effect (`DelayedPushBack` and
/// `DelayedRemove`). Hence, it is possible to safely add and remove elements
/// while traversing the container without invalidating iterators, and pointer
/// and
/// references to elements inside the container.
template <typename T>
class TransactionalVector {
 public:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  using value_type = T;

  /// This function is called during ROOT LinkDef generation.
  /// It adds a linkdef entry for each data member or base type.
  /// If this type is subclassed it also adds an entry of itself.
  // TODO link to documentation
  static void AddToLinkDef(std::set<LinkDefDescriptor>& entries) {
    AddAllLinkDefEntries<std::vector<T>>(entries, true);
  }

  TransactionalVector() {}

  /// Returns the vector's size. Uncommited changes are not taken into account
  size_t size() const {  // NOLINT
    return size_;
  }

  /// Safe method to add an element to this vector.
  /// Does not invalidate, iterators, pointers or references.
  /// Changes do not take effect until they are commited.
  /// @param  element that should be added to the vector
  /// @return index of the added element in `data_`. Will be bigger than
  ///         `size()`
  uint64_t DelayedPushBack(const T& element) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    data_.push_back(element);
    return data_.size() - 1;
  }

  /// Safe method to remove an element from this vector
  /// Does not invalidate, iterators, pointers or references.
  /// Changes do not take effect until they are commited.
  /// Upon commit removal has constant complexity @see Commit
  /// @param index remove element at the given index
  void DelayedRemove(size_t index) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    to_be_removed_.push_back(index);
  }

  /// This method commits changes made by `DelayedPushBack` and `DelayedRemove`.
  /// CAUTION: \n
  ///   * Commit invalidates pointers and references returned by
  ///     `DelayedPushBack`. \n
  ///   * If memory reallocations are required all pointers or references
  ///     into this container are invalidated\n
  /// One removal has constant complexity. If the element which should be
  /// removed is not the last element it is swapped with the last one.
  /// (CAUTION: this invalidates pointers and references to the last element)
  /// In the next step it can be removed in constant time using pop_back. \n
  void Commit() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // commit delayed push backs
    size_ = data_.size();
    // commit delayed removes
    // sort indices in descending order to prevent out of bounds accesses
    auto descending = [](auto a, auto b) { return a > b; };
    std::sort(to_be_removed_.begin(), to_be_removed_.end(), descending);
    for (size_t idx : to_be_removed_) {
      assert(idx < data_.size() && "Removed index outside array boundaries");
      if (idx < data_.size() - 1) { // idx points to last element
        // invalidates pointer of last element
        std::swap(data_[idx], data_[data_.size() - 1]);
        data_.pop_back();
      } else {  // idx does not point to last element
        data_.pop_back();
      }
      size_--;
    }
    to_be_removed_.clear();
  }

  /// Thread safe version of `std::vector::push_back`.
  /// Throws `std::logic_error` if there are uncommited delayed additions.
  void push_back(const T& element) {  // NOLINT
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (data_.size() == size_) {
      data_.push_back(element);
      size_++;
    } else {
      throw std::logic_error("There are uncommited delayed additions to this container");
    }
  }

  /// Thread safe version of std::vector::reserve
  void reserve(size_t new_capacity) {  // NOLINT
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    data_.reserve(new_capacity);
  }

  /// Thread-safe version of std::vector::clear
  void clear() {  // NOLINT
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    data_.clear();
    size_ = 0;
  }

  T& operator[](size_t index) { return data_[index]; }

  const T& operator[](size_t index) const { return data_[index]; }

  iterator begin() { return data_.begin(); }  // NOLINT

  iterator end() { return data_.begin() += size_; }  // NOLINT

  const_iterator cbegin() { return data_.cbegin(); }  // NOLINT

  const_iterator cend() { return data_.cbegin() += size_; }  // NOLINT

 private:
  std::recursive_mutex mutex_;  //!
  std::vector<T> data_;
  uint64_t size_ = 0;
  /// vector of indices with elements which should be removed
  std::vector<size_t> to_be_removed_;

  BDM_ROOT_CLASS_DEF(TransactionalVector, 1);  // TODO change to ClassDef
};

}  // namespace bdm

#endif  // TRANSACTIONAL_VECTOR_H_
