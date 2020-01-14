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

#include "TInterpreter.h"
#include "TPluginManager.h"
#include "TROOT.h"

namespace bdm {

VisualizationAdaptor *VisualizationAdaptor::Create(std::string adaptor) {
  TPluginHandler *h;
  VisualizationAdaptor *va = nullptr;
  std::string bdm_src_dir = std::string(std::getenv("BDM_SRC_DIR"));
  if ((h = gPluginMgr->FindHandler("VisualizationAdaptor", adaptor.c_str()))) {
    if (h->LoadPlugin() == -1) {
      Log::Error("VisualizationAdaptor::Create", "Was unable to load plugin '",
                 adaptor, "'!");
      return nullptr;
    }
    va = (VisualizationAdaptor *)h->ExecPlugin(0);
    Log::Info("VisualizationAdaptor::Create", "Loaded plugin '", adaptor,
              "' successfully!");
  } else {
    Log::Error(
        "VisualizationAdaptor::Create", "Unable to find plugin '", adaptor,
        "'. This is most likely because bdm.rootrc was not read properly.");
  }
  return va;
}

}  // namespace bdm
