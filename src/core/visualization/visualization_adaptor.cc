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

#include <string>

#include "TPluginManager.h"
#include "TROOT.h"

namespace bdm {

VisualizationAdaptor *VisualizationAdaptor::Create(std::string adaptor) {
  TPluginHandler *h;
  VisualizationAdaptor *va = nullptr;
  if ((h = gPluginMgr->FindHandler("VisualizationAdaptor", adaptor.c_str()))) {
    if (h->LoadPlugin() == -1) {
      Log::Warning("VisualizationAdaptor::Create",
                   "Was unable to load plugin '", adaptor, "'!");
      return nullptr;
    }
    va = (VisualizationAdaptor *)h->ExecPlugin(0);
    Log::Info("VisualizationAdaptor::Create", "Loaded plugin '", adaptor,
              "' successfully!");
  } else {
    Log::Warning(
        "VisualizationAdaptor::Create", "Unable to find plugin '", adaptor,
        "'. This is most likely because bdm.rootrc was not read properly.");
  }
  return va;
}

}  // namespace bdm
