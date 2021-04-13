// -----------------------------------------------------------------------------
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

#ifndef VERSION_H_
#define VERSION_H_

#include <cstdint>

/// Version string as obtained from `git describe --tags`
/// e.g.: v1.0.123-g1234abc
/// vMAJOR.MINOR.PATH-ADDITIONAL_COMMITS-gCOMMIT_ID
#define BDM_RELEASE "@VERSION@"

/// These macros can be used in the following way:
///
///    #if BDM_VERSION_CODE >= BDM_VERSION(1,0,0)
///       #include <newheader.h>
///    #else
///       #include <oldheader.h>
///    #endif
#define BDM_VERSION(major, minor, patch) \
  (static_cast<uint64_t>(major) << 32) + \
      (static_cast<uint64_t>(minor) << 16) + static_cast<uint64_t>(patch)

#define BDM_VERSION_CODE \
  BDM_VERSION(@VERSION_MAJOR @, @VERSION_MINOR @, @VERSION_PATCH @)

namespace bdm {

class Version {
 public:
  /// Returns version string as obtained from `git describe --tags`
  /// e.g.: v1.0.123-g1234abc
  /// vMAJOR.MINOR-PATCH-gCOMMIT_ID
  static const char *String() { return BDM_RELEASE; }
  static uint64_t Code() { return BDM_VERSION_CODE; }
};

}  // namespace bdm

#endif  // VERSION_H_
