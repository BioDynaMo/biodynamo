// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_FUNCTOR_H_
#define CORE_FUNCTOR_H_

namespace bdm {

// -----------------------------------------------------------------------------
template <typename TReturn, typename... TArgs>
class Functor {
 public:
  virtual ~Functor() {}
  virtual TReturn operator()(TArgs... args) = 0;
};

// -----------------------------------------------------------------------------
template <typename TLambda>
struct LambdaFunctor {};

template <typename TLambda, typename TReturn, typename... TArgs>
struct LambdaFunctor<TReturn (TLambda::*)(TArgs...) const> final
    : public Functor<TReturn, TArgs...> {
  TLambda lambda;

  LambdaFunctor(TLambda&& lambda) : lambda(lambda) {}
  LambdaFunctor(TLambda lambda) : lambda(lambda) {}
  LambdaFunctor(LambdaFunctor&& other) : lambda(std::move(other.lambda)) {}

  TReturn operator()(TArgs... args) override { return lambda(args...); }
};

// -----------------------------------------------------------------------------
template <typename TLambda>
LambdaFunctor<decltype(&TLambda::operator())> MakeFunctor(
    const TLambda& lambda) {
  LambdaFunctor<decltype(&TLambda::operator())> f(lambda);
  return f;
}

}  // namespace bdm

#endif  // CORE_FUNCTOR_H_
