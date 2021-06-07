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

#include <utility>

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
/// Together with the function `bdm::L2F` this class allows to define
/// new functors exactly were they are needed and doesn't require the definition
/// of a new class.
template <typename TLambda>
struct LambdaFunctor {};

/// \see `bdm::LambdaFunctor`
template <typename TLambda, typename TReturn, typename... TArgs>
struct LambdaFunctor<TReturn (TLambda::*)(TArgs...) const> final
    : public Functor<TReturn, TArgs...> {
  TLambda lambda;

  LambdaFunctor(const TLambda& lambda) : lambda(lambda) {}
  LambdaFunctor(const LambdaFunctor& other) : lambda(other.lambda) {}
  virtual ~LambdaFunctor() {}

  TReturn operator()(TArgs... args) override {
    return lambda(std::forward<TArgs>(args)...);
  }
};

/// \see `bdm::LambdaFunctor`
template <typename TLambda, typename TReturn, typename... TArgs>
struct LambdaFunctor<TReturn (TLambda::*)(TArgs...)> final
    : public Functor<TReturn, TArgs...> {
  TLambda lambda;

  LambdaFunctor(const TLambda& lambda) : lambda(lambda) {}
  LambdaFunctor(const LambdaFunctor& other) : lambda(other.lambda) {}
  virtual ~LambdaFunctor() {}

  TReturn operator()(TArgs... args) override {
    return lambda(std::forward<TArgs>(args)...);
  }
};

// -----------------------------------------------------------------------------
/// Wraps a lambda inside a LambdaFunctor with the same signature as the lambda.
/// Assume the following example using `L2F`
///
///     void PrintSmallNeighbors(Agent* agent, double threshold) {
///       auto functor = L2F([&](Agent* neighbor, double squared_distance)) {
///          if (neighbor->GetDiameter() < threshold) {
///            std::cout << neighbor->GetUid() << std::endl;
///          }
///       });
///       auto* ctxt = Simulation::GetActive()->GetExecutionContext();
///       ctxt->ForEachNeighbor(functor, *agent);
///     }
///
/// The base class of `functor` in the example above is
/// `Functor<void, Agent*, double>`\n
/// The wrapped lambda is allowed to capture variables. \n
/// Without bdm::LambdaFunctor and bdm::L2F the following code is needed
/// to achieve the same result as above. Notice the extra class `MyFunctor` that
/// has to be defined outside `PrintSmallNeighbors`.
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
///     void PrintSmallNeighbors(Agent* agent, double threshold) {
///       MyFunctor functor(threshold);
///       auto* ctxt = Simulation::GetActive()->GetExecutionContext();
///       ctxt->ForEachNeighbor(functor, *agent);
///     }
///
template <typename TLambda>
LambdaFunctor<decltype(&TLambda::operator())> L2F(const TLambda& l) {
  return LambdaFunctor<decltype(&TLambda::operator())>(l);
}

}  // namespace bdm

#endif  // CORE_FUNCTOR_H_
