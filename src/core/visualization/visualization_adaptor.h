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

#ifndef CORE_VIRTUALIZATION_VIRTUALIZATION_ADAPTOR_H_
#define CORE_VIRTUALIZATION_VIRTUALIZATION_ADAPTOR_H_

#include <string>

namespace bdm {

class VisualizationAdaptor {
 public:
  VisualizationAdaptor() {}

  static VisualizationAdaptor* Create(std::string adaptor);

  virtual ~VisualizationAdaptor() {}

  // To be implemented by the adaptor
  virtual void Visualize() = 0;
};

}  // namespace bdm

#endif  // CORE_VIRTUALIZATION_VIRTUALIZATION_ADAPTOR_H_
