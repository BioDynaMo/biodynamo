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

#include "core/visualization/catalyst_adaptor.h"

#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

namespace bdm {

vtkCPProcessor* CatalystAdaptor::g_processor_ = nullptr;

constexpr const char* CatalystAdaptor::kSimulationInfoJson;

std::atomic<uint64_t> CatalystAdaptor::counter_;

}  // namespace bdm

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)
