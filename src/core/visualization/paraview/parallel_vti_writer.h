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

#ifndef CORE_VISUALIZATION_PARAVIEW_PARALLEL_VTI_WRITER_H_
#define CORE_VISUALIZATION_PARAVIEW_PARALLEL_VTI_WRITER_H_

// std
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
// Paraview
#include <vtkImageData.h>
#include <vtkXMLImageDataWriter.h>

namespace bdm {

// -----------------------------------------------------------------------------
class VtiWriter : public vtkXMLImageDataWriter {
 public:
  vtkTypeMacro(VtiWriter, vtkXMLImageDataWriter);
  static VtiWriter* New();

  VtiWriter();

  void SetWholeExtent(const int* whole_extent);

  void WritePrimaryElementAttributes(std::ostream& os,
                                     vtkIndent indent) override;

 private:
  const int* whole_extent_;
};

// -----------------------------------------------------------------------------
class PvtiWriter {
 public:
  void Write(const std::string& folder, const std::string& file_prefix,
             const std::array<int, 6>& whole_extent,
             const std::vector<std::array<int, 6>>& piece_extents,
             vtkImageData* img, VtiWriter* vti);

 private:
  template <typename T>
  std::string ArrayToString(T* data, int length) {
    std::stringstream stream;
    stream << data[0];
    for (int i = 1; i < length; ++i) {
      stream << " " << data[i];
    }
    return stream.str();
  }
};

// -----------------------------------------------------------------------------
struct ParallelVtiWriter {
  void operator()(const std::string& folder, const std::string& file_prefix,
                  const std::vector<vtkImageData*>& images, uint64_t num_pieces,
                  const std::array<int, 6>& whole_extent,
                  const std::vector<std::array<int, 6>>& piece_extents) const;
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_PARALLEL_VTI_WRITER_H_
