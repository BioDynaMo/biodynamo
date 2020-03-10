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

#ifndef CORE_UTIL_JIT_H_
#define CORE_UTIL_JIT_H_

#include <string>
#include <vector>
#include <functional>

namespace bdm {

class JitForEachDataMemberFunctor {
public:
 JitForEachDataMemberFunctor(const std::string class_name,
                             const std::vector<std::string> dm_names,
                             const std::string functor_name,
                             const std::function<std::string()>& code_generation);

 void Compile();

 void* New(const std::string& parameter = "");

 template <typename TFunctor>
 TFunctor* New(const std::string& parameter = "") {
   return static_cast<TFunctor*>(New(parameter));
 }

 private:
   std::string functor_name_;
   std::function<std::string()> code_generator_;
};

}  // namespace bdm

#endif  // CORE_UTIL_JIT_H_
