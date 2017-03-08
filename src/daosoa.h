#ifndef DAOSOA_H_
#define DAOSOA_H_

#include <vector>
#include <type_traits>
#include "aosoa.h"
#include "backend.h"
#include "inline_vector.h"

namespace bdm {

template <template <typename> class T, typename Backend = VcBackend>
class daosoa {
 public:
  // soa of type T
  using value_type = T<Backend>;
  using iterator = typename std::vector<value_type>::iterator;
  using const_iterator = typename std::vector<value_type>::const_iterator;

  daosoa() {}
  explicit daosoa(size_t scalar_elements) {
    data_.reserve(scalar_elements / Backend::kVecLen +
                  (scalar_elements % Backend::kVecLen ? 1 : 0));
  }

  explicit daosoa(const value_type& cell) { data_.push_back(cell); }

  /// \brief returns the number of SOA elements in this container
  size_t vectors() const { return data_.size(); }

  /// this function assumes that only the last vector may not be fully
  /// initialized
  size_t elements() const {
    if (vectors() != 0) {
      return (vectors() - 1) * Backend::kVecLen +
             data_[vectors() - 1].Size();  // fixme Size vectors
    } else {
      return 0;
    }
  }

  // todo improve this comment
  /// \brief adds `value` to the vector
  /// only gets compiled if T == T1
  template <typename T1>
  typename std::enable_if<std::is_same<value_type, T1>::value>::type push_back(
      const T1& value) {
    data_.push_back(std::move(value));
    // fixme broken: what if i first push_back a scalar element and then
    // a vector one??
  }

  // todo improve this comment
  /// \brief adds `scalar value` to the vector
  /// only gets compiled if T != T1 && T1 == ScalarBackend
  template <typename T1>
  typename std::enable_if<is_scalar<T1>::value &&
                          !std::is_same<value_type, T1>::value>::type
  push_back(const T1& value) {
    // this implementation is only allowed if T != T1 && T1 == ScalarBackend
    static_assert(
        is_scalar<T1>::value && !std::is_same<value_type, T1>::value,
        "push_back of a non scalar type on a scalar type is not supported");
    if (data_.size() == 0) {
      value_type v;
      v.SetUninitialized();
      data_.emplace_back(std::move(v));
    }
    auto last = &data_[data_.size() - 1];
    if (last->is_full()) {
      value_type v1;
      v1.SetUninitialized();
      data_.emplace_back(std::move(v1));
      last = &data_[data_.size() - 1];
    }
    last->Append(value);
  }

  void Gather(const InlineVector<int, 8>& indexes,
              aosoa<T, Backend>* ret) const {
    const size_t scalars = indexes.size();
    std::size_t n_vectors =
        scalars / Backend::kVecLen + (scalars % Backend::kVecLen ? 1 : 0);
    std::size_t remaining = scalars % Backend::kVecLen;

    ret->SetSize(n_vectors);
    for (std::size_t i = 0; i < n_vectors; i++) {
      if (i != n_vectors - 1 || remaining == 0) {
        (*ret)[i].SetSize(Backend::kVecLen);
      } else {
        (*ret)[i].SetSize(remaining);
      }
    }

    size_t counter = 0;
    value_type* dest = nullptr;
    for (size_t i = 0; i < scalars; i++) {
      int idx = indexes[i];
      size_t vector_idx = idx / Backend::kVecLen;
      size_t vec_el_idx = idx % Backend::kVecLen;
      size_t dest_idx = counter % Backend::kVecLen;
      if (dest_idx == 0) {
        dest = &((*ret)[counter / Backend::kVecLen]);
      }
      data_[vector_idx].CopyTo(vec_el_idx, dest_idx, dest);
      counter++;
    }
  }

  /// \brief returns scalar representation of element at index
  T<ScalarBackend> GetScalar(size_t index) const {
    // fixme if this is ScalarBackend
    size_t vector_idx = index / Backend::kVecLen;
    size_t vec_el_idx = index % Backend::kVecLen;
    return data_[vector_idx].Get(vec_el_idx);
  }

  void SetScalar(size_t index, const T<ScalarBackend>& value) {
    // fixme if this is ScalarBackend
    size_t vector_idx = index / Backend::kVecLen;
    size_t vec_el_idx = index % Backend::kVecLen;
    return data_[vector_idx].Set(vec_el_idx, value);
  }

  Vc_ALWAYS_INLINE value_type& operator[](std::size_t index) {
    return data_[index];
  }
  Vc_ALWAYS_INLINE const value_type& operator[](std::size_t index) const {
    return data_[index];
  }

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }

  const_iterator begin() const { return data_.cbegin(); }
  const_iterator end() const { return data_.cend(); }

 private:
  std::vector<value_type, Vc::Allocator<value_type> > data_;
};

}  // namespace bdm

#endif  // DAOSOA_H_
