// i----------------------------------------------------------------------------
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

#include <string>
#include <unordered_map>

#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/visualization/visualization_adaptor.h"

#include "TInterpreter.h"
#include "TPluginManager.h"
#include "TROOT.h"

namespace bdm {

// This map keeps track of whether plugins (the keys) have been loaded, and
// registers their corresponding plugin handler pointer (the values). This way
// we can reuse the plugin handler without dynamically loading the shared
// library more than once
static std::unordered_map<std::string, TPluginHandler *> loaded_;

VisualizationAdaptor *VisualizationAdaptor::Create(std::string adaptor) {
  auto *param = Simulation::GetActive()->GetParam();
  if (!(param->insitu_visualization || param->export_visualization)) {
    return nullptr;
  }
  VisualizationAdaptor *va = nullptr;
  bool first_try = !loaded_.count(adaptor);
  // If this is the first time we try to load `adaptor`
  if (first_try) {
    // Try to find plugin handler in etc/plugins
    auto *h = gPluginMgr->FindHandler("VisualizationAdaptor", adaptor.c_str());
    if (h) {
      // Try to load the plugin (dynamically load shared library)
      if (h->LoadPlugin() == 0) {
        // Register the plugin handler for future use
        loaded_[adaptor] = h;
        va = reinterpret_cast<VisualizationAdaptor *>(h->ExecPlugin(0));
        return va;
      } else {
        Log::Error("VisualizationAdaptor::Create",
                   "Was unable to load plugin '", adaptor, "'!");
        return nullptr;
      }
    } else {
      Log::Error(
          "VisualizationAdaptor::Create", "Unable to find plugin '", adaptor,
          "'. This is most likely because bdm.rootrc was not read properly.");
    }
  } else {  // If we already tried loading this plugin, we avoid dynamically
            // loading the shared library again
    if (loaded_[adaptor]) {  // If we successfully loaded the plugin before
      va = reinterpret_cast<VisualizationAdaptor *>(
          loaded_[adaptor]->ExecPlugin(0));
      return va;
    } else {  // If we failed to load the plugin before
      return nullptr;
    }
  }

  Log::Info("VisualizationAdaptor::Create", "Loaded plugin '", adaptor,
            "' successfully!");
  return va;
}

}  // namespace bdm
