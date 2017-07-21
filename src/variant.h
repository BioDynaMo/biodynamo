#ifndef VARIANT_H_
#define VARIANT_H_

#include <Rtypes.h>
#include <TBuffer.h>
#include <iostream>
#include <tuple>

#include "mpark/variant.hpp"
#include "root_util.h"
#include "tuple_util.h"

namespace bdm {

/// Wrapper for mpark::visit
template <typename TFunction, typename TVariant>
void visit(TFunction&& function, TVariant&& variant_wrapper) {
  mpark::visit(function, variant_wrapper.data_);
}

/// Wrapper for mpark::get_if
template <typename T, typename TVariant>
const T* get_if(TVariant* variant_wrapper) {
  return mpark::get_if<T>(&(variant_wrapper->data_));
}

// TODO rename
/// Wrapper for mpark::variant.
/// Necessary, because mpark::variant canno be written to a ROOT file.
/// Therefore, it is wrapped in this object that has a custom streamer.
template <typename... Types>
class variant {
 public:
  template <typename T>
  variant(const T& value) : data_(value) {}

  variant() {}

  virtual ~variant() {}

  template <typename T>
  variant<Types...>& operator=(const T& value) {
    data_ = value;
    return *this;
  }

 private:
  mpark::variant<Types...> data_;

  // friend visit and get_if
  template <typename TFunction, typename TVariant>
  friend void visit(TFunction&& function, TVariant&& variant_wrapper);

  template <typename T, typename TVariant>
  friend const T* get_if(TVariant* variant_wrapper);

  BDM_TEMPLATE_CLASS_DEF_CUSTOM_STREAMER(variant, 1);
};

/// Helper functor to write the contents of a variant to a TBuffer
template <typename... Types>
struct StreamerWriteFunctor {
  StreamerWriteFunctor(TBuffer& buffer) : buffer_(buffer) {}

  template <typename T>
  typename std::enable_if<std::is_fundamental<T>::value>::type operator()(
      T& t) {  // NOLINT
    auto type_id = GetIndex<T, Types...>();
    buffer_ << type_id;
    buffer_ << t;
  }

  template <typename T>
  typename std::enable_if<!std::is_fundamental<T>::value>::type operator()(
      T& t) {  // NOLINT
    auto type_id = GetIndex<T, Types...>();
    buffer_ << type_id;
    buffer_.WriteClassBuffer(T::Class(), &t);
  }

 private:
  TBuffer& buffer_;
};

/// Helper functor to read the contents of a variant from a TBuffer
template <typename TVariant>
struct StreamerReadFunctor {
  StreamerReadFunctor(TBuffer& buffer, TVariant* variant)
      : buffer_(buffer), variant_(variant) {}

  // version for fundamental type
  template <typename T>
  typename std::enable_if<std::is_fundamental<T>::value>::type operator()(
      T& value) {
    buffer_ >> value;
    *variant_ = value;
  }

  // version for non fundamental types
  template <typename T>
  typename std::enable_if<!std::is_fundamental<T>::value>::type operator()(
      T& value) {
    buffer_.WriteClassBuffer(T::Class(), &value);
    *variant_ = value;
  }

 private:
  TBuffer& buffer_;
  TVariant* variant_;
};

// Custom streamer for bdm::variant<Types...> required for ROOT IO
template <typename... Types>
inline void variant<Types...>::Streamer(TBuffer& R__b) {
  if (R__b.IsReading()) {
    size_t type_id;
    R__b >> type_id;

    static std::tuple<Types...> tuple;
    bdm::Apply(&tuple, type_id,
               StreamerReadFunctor<decltype(data_)>(R__b, &data_));
  } else {
    mpark::visit(StreamerWriteFunctor<Types...>(R__b), data_);
  }
}

}  // namespace bdm

#endif  // VARIANT_H_
