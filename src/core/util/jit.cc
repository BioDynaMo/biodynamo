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

#include <stack>
#include <iostream> // FIXME remove

#include <TInterpreter.h>
#include <TClassTable.h>
#include <TDataMember.h>
#include <TClass.h>
#include <TList.h>

#include "core/util/jit.h"
#include "core/util/string.h"
#include "core/util/log.h"

namespace bdm {

// FIXME add tests
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

// FIXME add tests
std::vector<TDataMember*> FindDataMemberSlow(TClass* tclass, const std::string& data_member) {
  std::vector<TDataMember*> ret_val;

  bool dm_has_scope = data_member.find("::") != std::string::npos;
  std::string class_name = "";
  std::string dm_only_name = data_member;
  if (dm_has_scope) {
    auto idx = data_member.find_last_of("::");
    class_name = data_member.substr(0, idx - 1);
    dm_only_name = data_member.substr(idx + 1, data_member.size());
  }

  std::stack<TClass*> tc_stack;
  tc_stack.push(tclass);

  while(tc_stack.size() != 0) {
    auto* current_tc = tc_stack.top();
    tc_stack.pop();
    for (const auto&& base : *current_tc->GetListOfBases()) {
      auto* tbase = TClassTable::GetDict(base->GetName())();
      tc_stack.push(tbase);
    }

    for(int i = 0; i < current_tc->GetListOfDataMembers()->GetSize(); ++i) {
      auto* dm = static_cast<TDataMember*>(current_tc->GetListOfDataMembers()->At(i));
      if (dm_has_scope) {
        if (dm_only_name.compare(dm->GetName()) == 0 && EndsWith(std::string(current_tc->GetName()), class_name)) {
          ret_val.push_back(dm);
        }
      } else {
        if (data_member.compare(dm->GetName()) == 0) {
          ret_val.push_back(dm);
        }
      }
    }
  }

  return ret_val;
}

JitForEachDataMemberFunctor::JitForEachDataMemberFunctor(TClass* tclass,
                           const std::vector<std::string> dm_names,
                           const std::string functor_name,
                           const std::function<std::string(const std::vector<TDataMember*>&)>& code_generator)
                           : functor_name_(functor_name)
                           , code_generator_(code_generator) {

  data_members_.reserve(dm_names.size());
  for(auto& dm : dm_names) {
    auto candidates = FindDataMemberSlow(tclass, dm);
    if (candidates.size() == 1) {
      data_members_.push_back(candidates[0]);
    } else if (candidates.size() == 0) {
      // FIXME message
      Log::Fatal("JitForEachDataMemberFunctor::JitForEachDataMemberFunctor", "Could not find data member");
    } else {
      // FIXME message
      Log::Fatal("JitForEachDataMemberFunctor::JitForEachDataMemberFunctor", "Data member name is ambigous");
    }
  }
}

void JitForEachDataMemberFunctor::Compile() {
  gInterpreter->ProcessLineSynch(code_generator_(data_members_).c_str());
}

void* JitForEachDataMemberFunctor::New(const std::string& parameter) {
  auto cmd = Concat("new bdm::", functor_name_, "(", parameter, ")");
  return reinterpret_cast<void*>(gInterpreter->Calc(cmd.c_str()));
}

}  // namespace bdm
