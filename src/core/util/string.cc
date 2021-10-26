// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/util/string.h"
#include <iostream>
namespace bdm {

std::vector<std::string> Split(const std::string& s,
                               const std::string& delimiter) {
  std::vector<std::string> result;
  uint64_t pos = 0;
  uint64_t last = 0;

  while ((pos = s.find(delimiter, last)) != std::string::npos) {
    result.push_back(s.substr(last, pos - last));
    last = pos + delimiter.length();
  }
  result.push_back(s.substr(last));
  return result;
}

}  // namespace bdm
