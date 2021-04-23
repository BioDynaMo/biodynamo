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
/// Subclass of `bdm::Functor` that wraps a lambda with the same signature and
/// forwards the call to the lambda. \n
/// Together with the function `bdm::MakeFunctor` this class allows to define
/// new functors exactly were they are needed and doesn't require the definition
/// of a new class.
template <typename TLambda>
struct LambdaFunctor {};

/// \see `bdm::LambdaFunctor`
template <typename TLambda, typename TReturn, typename... TArgs>
struct LambdaFunctor<TReturn (TLambda::*)(TArgs...) const> final
    : public Functor<TReturn, TArgs...> {
  TLambda lambda;

  LambdaFunctor(TLambda&& lambda) : lambda(lambda) {}
  LambdaFunctor(TLambda lambda) : lambda(lambda) {}
  LambdaFunctor(LambdaFunctor&& other) : lambda(std::move(other.lambda)) {}
  virtual ~LambdaFunctor() {}

  TReturn operator()(TArgs... args) override { return lambda(args...); }
};

// -----------------------------------------------------------------------------
/// Wraps a lambda inside a LambdaFunctor with the same signature as the lambda.
/// Assume the following example using `MakeFunctor`
///
///     void SomeFunction(...) {
///       ...
///       double threshold = 10;
///       auto functor = MakeFunctor([&](Agent* neighbor, double
///       squared_distance)) {
///          if (neighbor->GetDiameter() < threshold) {
///            std::cout << agent->GetUid() << std::endl;
///          }
///       });
///       ctxt->ForEachNeighbor(functor, *agent);
///       ...
///     }
///
///  The base class of `functor` in the example above is
/// `Functor<void, Agent*, double>`\n
/// The wrapped lambda is allowed to capture variables. \n
/// Without bdm::LambdaFunctor and bdm::MakeFunctor the following code is needed
/// to achieve the same result as above. Notice the extra class `MyFunctor` that
/// has to be defined outside `SomeFunction`.
///
///     class MyFunctor : public Functor<void, Agent*, double> {
///      public:
///       MyFunctor(double threshold) : threshold_(threshold) {}
///       virtual ~MyFunctor() {}
///       void operator()(Agent* neighbor, double squared_distance) override {
///         if (neighbor->GetDiameter() < threshold_) {
///           std::cout << agent->GetUid() << std::endl;
///         }
///       }
///      private:
///       double threshold_;
///     };
///
///     void SomeFunction(...) {
///       ...
///       double threshold = 10;
///       MyFunctor functor(threshold);
///       ctxt->ForEachNeighbor(functor, *agent);
///       ...
///     }
///
template <typename TLambda>
LambdaFunctor<decltype(&TLambda::operator())> MakeFunctor(
    const TLambda& lambda) {
  LambdaFunctor<decltype(&TLambda::operator())> f(lambda);
  return f;
}

}  // namespace bdm

#endif  // CORE_FUNCTOR_H_
