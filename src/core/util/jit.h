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

class TClass;
class TDataMember;

namespace bdm {

/// Iterates over all entries in `TClassTable` and returns a list of candidate
/// TClass* that match the given class name.
/// \param class_name does not have to be fully qualified
///        (e.g. `Cell` instead of `bdm::Cell`). \n
///        However, `Cell` will also match e.g "bdm::foo::Cell"
/// \return multiple values if class_name is ambigous and multiple classes
///         were found in different namespaces
std::vector<TClass*> FindClassSlow(const std::string& class_name);

class JitForEachDataMemberFunctor {
public:
 JitForEachDataMemberFunctor(const std::string class_name,
                             const std::vector<std::string> dm_names,
                             const std::string functor_name,
                             const std::function<std::string(const std::vector<TDataMember*>&)>& code_generation);

 void Compile();

 void* New(const std::string& parameter = "");

 template <typename TFunctor>
 TFunctor* New(const std::string& parameter = "") {
   return static_cast<TFunctor*>(New(parameter));
 }

 private:
   std::string functor_name_;
   std::vector<TDataMember*> data_members_;
   std::function<std::string(const std::vector<TDataMember*>&)> code_generator_;
};

}  // namespace bdm

#endif  // CORE_UTIL_JIT_H_
