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

#ifndef VERSION_H_
#define VERSION_H_

#include <cstdint>

/// Version string as obtained from `git describe --tags`
/// e.g.: v0.1.0-10-g91aade4
/// vMAJOR.MINOR.PATH-ADDITIONAL_COMMITS-gCOMMIT_ID
#define BDM_RELEASE "v0.1.0-363-gdcda977"

/// These macros can be used in the following way:
///
///    #if BDM_VERSION_CODE >= BDM_VERSION(1,0,0,0)
///       #include <newheader.h>
///    #else
///       #include <oldheader.h>
///    #endif
#define BDM_VERSION(major, minor, patch, additional_commits) \
  (static_cast<uint64_t>(major) << 48) +                     \
      (static_cast<uint64_t>(minor) << 32) +                 \
      (static_cast<uint64_t>(patch) << 16) +                 \
      static_cast<uint64_t>(additional_commits)
#define BDM_VERSION_CODE                                         \
  BDM_VERSION(0, 1, 0, \
              363)

namespace bdm {

class Version {
 public:
  /// Returns version string as obtained from `git describe --tags`
  /// e.g.: v0.1.0-10-g91aade4`
  /// vMAJOR.MINOR.PATCH-ADDITIONAL_COMMITS-gCOMMIT_ID
  static const char *String() { return "v0.1.0-363-gdcda977"; }
  static uint64_t Code() { return BDM_VERSION_CODE; }
};

}  // namespace bdm

#endif  // VERSION_H_
