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

#include <TInterpreter.h>
#include <TClassTable.h>

#include "core/util/jit.h"
#include "core/util/string.h"

namespace bdm {

JitForEachDataMemberFunctor::JitForEachDataMemberFunctor(const std::string class_name,
                           const std::vector<std::string> dm_names,
                           const std::string functor_name,
                           const std::function<std::string()>& code_generator)
                           : functor_name_(functor_name)
                           , code_generator_(code_generator) {

}

void JitForEachDataMemberFunctor::Compile() {
  gInterpreter->ProcessLineSynch(code_generator_().c_str());
}



void* JitForEachDataMemberFunctor::New(const std::string& parameter) {
  auto cmd = Concat("new bdm::", functor_name_, "(", parameter, ")");
  return reinterpret_cast<void*>(gInterpreter->Calc(cmd.c_str()));
}

}  // namespace bdm
