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

#ifndef CORE_FUNCTOR_H_
#define CORE_FUNCTOR_H_

namespace bdm {

template <typename TReturn, typename... TParameter>
struct Functor {
  virtual ~Functor() {}
  virtual void operator()(TParameter... parameter) = 0;
};

}  // namespace bdm

#endif  // CORE_FUNCTOR_H_
