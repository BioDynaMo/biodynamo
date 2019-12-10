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

#ifndef CORE_VISUALIZATION_CATALYST_SO_VISITOR_H_
#define CORE_VISUALIZATION_CATALYST_SO_VISITOR_H_

// check for ROOTCLING was necessary, due to ambigous reference to namespace
// detail when using ROOT I/O
#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

#include <string>

#include "core/container/math_array.h"
#include "core/scheduler.h"
#include "core/sim_object/so_visitor.h"
#include "core/simulation.h"
#include "core/visualization/catalyst_helper.h"

namespace bdm {

/// This simulation object visitor is used to extract data from simulation
/// objects. It also creates the required vtk data structures and resets them
/// at the beginning of each iteration.
class CatalystSoVisitor : public SoVisitor {
 public:
  explicit CatalystSoVisitor(VtkSoGrid* so_grid);
  virtual ~CatalystSoVisitor();

  void Visit(const std::string& dm_name, size_t type_hash_code,
             const void* data) override;

  void Double(const std::string& dm_name, const void* d);

  void MathArray3(const std::string& dm_name, const void* d);

  void Int(const std::string& dm_name, const void* d);

  void Uint64T(const std::string& dm_name, const void* d);

  void Int3(const std::string& dm_name, const void* d);

 private:
  struct ParaViewImpl;
  std::unique_ptr<ParaViewImpl> impl_;
};

}  // namespace bdm

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

#endif  // CORE_VISUALIZATION_CATALYST_SO_VISITOR_H_
