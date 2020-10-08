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

#ifndef CORE_VISUALIZATION_PARAVIEW_PARALLEL_VTU_WRITER_H_
#define CORE_VISUALIZATION_PARAVIEW_PARALLEL_VTU_WRITER_H_

// std
#include <string>
#include <vector>
// Paraview
#include <vtkUnstructuredGrid.h>

namespace bdm {

struct ParallelVtuWriter {
  void operator()(const std::string& folder, const std::string& file_prefix,
                  const std::vector<vtkUnstructuredGrid*>& grids) const;
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_PARALLEL_VTU_WRITER_H_
