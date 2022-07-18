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

#include "core/util/root.h"
#include <TClassTable.h>
#include <TDataMember.h>
#include <TList.h>
#include <iomanip>
#include <iostream>

namespace bdm {

// -----------------------------------------------------------------------------
void PrintDataMemberInfo(const std::string& class_name) {
  auto* dm_list =
      TClassTable::GetDict(class_name.c_str())()->GetListOfDataMembers();
  std::cout << std::setw(40) << "Data member name"
            << "\tOffset\tSize\tType" << std::endl;
  for (auto el : *dm_list) {
    auto* dm = static_cast<TDataMember*>(el);
    std::cout << std::setw(40) << dm->GetName() << "\t" << dm->GetOffset()
              << "\t" << dm->GetUnitSize() << "\t" << dm->GetFullTypeName()
              << std::endl;
  }
}

}  // namespace bdm
