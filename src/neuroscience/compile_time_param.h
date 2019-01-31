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

#ifndef NEUROSCIENCE_COMPILE_TIME_PARAM_H_
#define NEUROSCIENCE_COMPILE_TIME_PARAM_H_

#include "core/param/compile_time_param.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/neuron_soma.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

/// Default compile time parameter for neuroscience module.
/// Users need to specify the type of NeuronSoma and NeuriteElement they are
/// using.
template <typename TBackend = Soa>
struct DefaultCTParam {
  using NeuronSoma = ::bdm::experimental::neuroscience::NeuronSoma;
  using NeuriteElement = ::bdm::experimental::neuroscience::NeuriteElement;

  // must be empty. otherwise ambigous with other specializations
  template <typename TSimObject, int Dummy>
  struct CTMap {};

  BDM_DEFAULT_CTPARAM_FOR(NeuronSoma){};
  BDM_DEFAULT_CTPARAM_FOR(NeuriteElement){};
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_COMPILE_TIME_PARAM_H_
