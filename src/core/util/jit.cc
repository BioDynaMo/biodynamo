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

#include <iostream> // FIXME remove

#include <TInterpreter.h>
#include <TClassTable.h>
#include <TDataMember.h>
#include <TClass.h>
#include <TList.h>

#include "core/util/jit.h"
#include "core/util/string.h"

namespace bdm {

std::vector<TClass*> FindClassSlow(const std::string& class_name) {
  bool cn_has_scope = class_name.find("::") != std::string::npos;
  std::string cn_with_scope_prefix = std::string("::") + class_name;
  char* current = 0;
  uint64_t idx = 0;
  std::vector<TClass*> tclasses;
  while ((current = TClassTable::At(idx++)) != nullptr) {
    std::string scurrent(current);
    if (scurrent.find("::") == std::string::npos) {
      // current does not contain a scope operator -> full match
      if (std::string(current).compare(class_name) == 0) {
        tclasses.push_back(TClassTable::GetDict(current)());
      }
    } else {
      if (cn_has_scope) {
        if (EndsWith(scurrent, class_name)) {
          tclasses.push_back(TClassTable::GetDict(current)());
        }
      } else {
        if (EndsWith(scurrent, cn_with_scope_prefix)) {
          tclasses.push_back(TClassTable::GetDict(current)());
        }
      }
    }
  }
  return tclasses;
}

JitForEachDataMemberFunctor::JitForEachDataMemberFunctor(const std::string class_name,
                           const std::vector<std::string> dm_names,
                           const std::string functor_name,
                           const std::function<std::string(const std::vector<TDataMember*>&)>& code_generator)
                           : functor_name_(functor_name)
                           , code_generator_(code_generator) {

  auto* tclass = TClassTable::GetDict("bdm::Cell")();
  data_members_.resize(2);
  data_members_[0] = static_cast<TDataMember*>(tclass->GetListOfDataMembers()->At(4));
  data_members_[1] = static_cast<TDataMember*>(tclass->GetListOfDataMembers()->At(6));
}

void JitForEachDataMemberFunctor::Compile() {
  gInterpreter->ProcessLineSynch(code_generator_(data_members_).c_str());
}

void* JitForEachDataMemberFunctor::New(const std::string& parameter) {
  auto cmd = Concat("new bdm::", functor_name_, "(", parameter, ")");
  return reinterpret_cast<void*>(gInterpreter->Calc(cmd.c_str()));
}

}  // namespace bdm
