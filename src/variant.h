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

#ifndef VARIANT_H_
#define VARIANT_H_

#include <Rtypes.h>
#include <TBuffer.h>
#include <iostream>
#include <tuple>

#include "log.h"
#include "mpark/variant.hpp"
#include "root_util.h"
#include "tuple_util.h"

namespace bdm {

/// Use same interface for bdm::Variant as std::variant
/// \see std::visit
template <typename TFunction, typename TVariant>
void visit(TFunction&& function, TVariant&& variant_wrapper) {  // NOLINT
  variant_wrapper.Visit(function);
}

/// Use same interface for bdm::Variant as std::variant
/// \see std::get_if
template <typename T, typename TVariant>
T* get_if(TVariant* variant_wrapper) {  // NOLINT
  return variant_wrapper->template GetIf<T>();
}

/// Use same interface for bdm::Variant as std::variant
/// \see std::get_if
template <typename T, typename TVariant>
const T* get_if(const TVariant* variant_wrapper) {  // NOLINT
  return variant_wrapper->template GetIf<T>();
}

/// Wrapper for mpark::variant.
/// Necessary, because mpark::variant canno be written to a ROOT file.
/// Therefore, it is wrapped in this object that has a custom streamer.
template <typename... Types>
class Variant {
 public:
  template <typename T>
  Variant(  // NOLINT
      const T& value,
      typename std::enable_if<GetIndex<T, Types...>() != -1>::type* p = nullptr)
      : data_(value) {}

  template <typename T>
  Variant(const T& value,  // NOLINT
          typename std::enable_if<GetIndex<T, Types...>() == -1>::type* p =
              nullptr) {
    Log::Fatal(
        "Variant",
        "You called the constructor with a type that is not in Types...");
  }

  Variant() {}

  virtual ~Variant() {}

  template <typename T>
  typename std::enable_if<GetIndex<T, Types...>() != -1,
                          Variant<Types...>&>::type
  operator=(const T& value) {
    data_ = value;
    return *this;
  }

  template <typename T>
  typename std::enable_if<GetIndex<T, Types...>() == -1,
                          Variant<Types...>&>::type
  operator=(const T& value) {
    Log::Fatal("Variant",
               "You called the assignment operator with a type that is not in "
               "Types...");
    return *this;
  }

  template <typename T>
  typename std::enable_if<GetIndex<T, Types...>() != -1, T*>::type GetIf() {
    return mpark::get_if<T>(&(data_));
  }

  template <typename T>
  typename std::enable_if<GetIndex<T, Types...>() == -1, T*>::type GetIf() {
    Log::Fatal("Variant",
               "You called GetIf with a type that is not in Types...");
    return nullptr;
  }

  template <typename T>
  typename std::enable_if<GetIndex<T, Types...>() != -1, const T*>::type GetIf()
      const {
    return mpark::get_if<T>(&(data_));
  }

  template <typename T>
  typename std::enable_if<GetIndex<T, Types...>() == -1, const T*>::type GetIf()
      const {
    Log::Fatal("Variant",
               "You called `GetIf const` with a type that is not in Types...");
    return nullptr;
  }

  template <typename TFunction>
  void Visit(TFunction&& function) {
    mpark::visit(function, data_);
  }

 private:
  mpark::variant<Types...> data_;

  // friend visit and get_if
  template <typename TFunction, typename TVariant>
  friend void visit(TFunction&& function,  // NOLINT
                    TVariant&& variant_wrapper);

  template <typename T, typename TVariant>
  friend T* get_if(TVariant* variant_wrapper);  // NOLINT

  template <typename T, typename TVariant>
  friend const T* get_if(const TVariant* variant_wrapper);  // NOLINT

  BDM_TEMPLATE_CLASS_DEF_CUSTOM_STREAMER(Variant, 1);
};

/// Helper functor to write the contents of a Variant to a TBuffer
template <typename... Types>
struct StreamerWriteFunctor {
  explicit StreamerWriteFunctor(TBuffer* buffer) : buffer_(buffer) {}

  template <typename T>
  typename std::enable_if<std::is_fundamental<T>::value>::type operator()(
      T& t) {  // NOLINT
    auto type_id = GetIndex<T, Types...>();
    *buffer_ << type_id;
    *buffer_ << t;
  }

  template <typename T>
  typename std::enable_if<!std::is_fundamental<T>::value>::type operator()(
      T& t) {  // NOLINT
    int type_id = GetIndex<T, Types...>();
    *buffer_ << type_id;
    buffer_->WriteClassBuffer(T::Class(), &t);
  }

 private:
  TBuffer* buffer_;
};

/// Helper functor to read the contents of a Variant from a TBuffer
template <typename TVariant>
struct StreamerReadFunctor {
  StreamerReadFunctor(TBuffer* buffer, TVariant* variant)
      : buffer_(buffer), variant_(variant) {}

  // version for fundamental type
  template <typename T>
  typename std::enable_if<std::is_fundamental<T>::value>::type operator()(
      T* value) {
    *buffer_ >> *value;
    *variant_ = *value;
  }

  // version for non fundamental types
  template <typename T>
  typename std::enable_if<!std::is_fundamental<T>::value>::type operator()(
      T* value) {
    buffer_->WriteClassBuffer(T::Class(), value);
    *variant_ = *value;
  }

 private:
  TBuffer* buffer_;
  TVariant* variant_;
};

// Custom streamer for bdm::Variant<Types...> required for ROOT IO
template <typename... Types>
inline void Variant<Types...>::Streamer(TBuffer& R__b) {  // NOLINT
  if (R__b.IsReading()) {
    int type_id;
    R__b >> type_id;

    static std::tuple<Types...> kTuple;
    bdm::Apply(&kTuple, type_id,
               StreamerReadFunctor<decltype(data_)>(&R__b, &data_));
  } else {
    mpark::visit(StreamerWriteFunctor<Types...>(&R__b), data_);
  }
}

}  // namespace bdm

#endif  // VARIANT_H_
