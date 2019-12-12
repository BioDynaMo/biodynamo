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

#include "core/visualization/visualization_adaptor.h"
#include "core/util/log.h"

#include <sstream>
#include <string>

#include "TPluginManager.h"
#include "TROOT.h"

namespace bdm {

VisualizationAdaptor *VisualizationAdaptor::Create(std::string adaptor) {
  TPluginHandler *h;
  VisualizationAdaptor *va = nullptr;

  if ((h = gPluginMgr->FindHandler("VisualizationAdaptor", adaptor.c_str()))) {
    if (h->LoadPlugin() == -1) {
      std::stringstream ss;
      ss << "Was unable to load plugin '" << adaptor << "'!" << std::endl;
      Log::Warning("VisualizationAdaptor::Visualize", ss.str());
      return nullptr;
    }
    va = (VisualizationAdaptor *)h->ExecPlugin(0);
  }
  return va;
}

}  // namespace bdm
