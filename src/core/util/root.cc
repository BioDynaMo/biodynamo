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

#if defined(USE_DICT)

#include <TROOT.h>
#include <experimental/filesystem>

#include "core/util/log.h"
#include "core/util/proc.h"
#include "core/util/root.h"
#include "core/util/string.h"

namespace fs = std::experimental::filesystem;

namespace bdm {

bool LoadExecutableDictIntoCling() {
  auto exe_dir = GetExecutableDirectory();
  auto exe_name = GetExecutableName();
  // when running inside the ROOT interpreter don't attempt to load
  // a corresponding dictionary. It does not exist.
  if (exe_dir == Concat(std::getenv("ROOTSYS"), "/bin/") &&
      exe_name == "root.exe") {
    return false;
  }

#ifdef LINUX
  auto so_ending = ".so";
#else   // APPLE
  auto so_ending = ".dylib";
#endif  // LINUX
  auto libstr = Concat(exe_dir, "lib", exe_name, "-dict", so_ending);
  if (!fs::exists(libstr)) {
    // special case for biodynamo-unit-test
    libstr = Concat(exe_dir, "../lib/lib", exe_name, "-dict", so_ending);
    if (!fs::exists(libstr)) {
      Log::Warning("LoadExecutableDictIntoCling", "Couldn't find dictionary.");
    }
    return false;
  }
  gROOT->ProcessLine(Concat("R__LOAD_LIBRARY(", libstr, ")").c_str());
  return true;
}

const bool RunAtStartup::value = LoadExecutableDictIntoCling();

}  // namespace bdm

#endif  // USE_DICT
