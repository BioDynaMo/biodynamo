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

#ifndef COMPILE_TIME_PARAM_H_
#define COMPILE_TIME_PARAM_H_

#include "backend.h"
#include "biology_module_util.h"
#include "cell.h"
#include "variadic_template_parameter_util.h"
#include "variant.h"

namespace bdm {

/// \brief Defines default compile time parameters
/// Values can be overwritten by subclassing it.
/// `struct bdm::CompileTimeParam` has been forward declared by classes using
/// compile time parameters. This struct must be defined -- e.g. by using
/// `BDM_DEFAULT_COMPILE_TIME_PARAM()`
/// NB Can't be used in tests because CompileTimeParam is hardcoded in Self
/// alias
/// @tparam TBackend required to use simulation objects with different Backend
template <typename TBackend = Soa>
struct DefaultCompileTimeParam {
  template <typename TTBackend>
  using Self = CompileTimeParam<TTBackend>;
  using Backend = TBackend;

  /// Defines backend used in ResourceManager
  using SimulationBackend = Soa;
  using BiologyModules = Variant<NullBiologyModule>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

/// Macro which sets compile time parameter to DefaultCompileTimeParam
/// Caution: This call must be made from namespace `::bdm`. Otherwise,
/// the forward declared `struct bdm::CompileTimeParam` will not be defined.
#define BDM_DEFAULT_COMPILE_TIME_PARAM() \
  struct CompileTimeParam : public DefaultCompileTimeParam<> {};

}  // namespace bdm

#endif  // COMPILE_TIME_PARAM_H_
