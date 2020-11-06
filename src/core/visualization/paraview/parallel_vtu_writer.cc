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

#include "core/visualization/paraview/parallel_vtu_writer.h"
// std
#include <fstream>
#include <sstream>
// Paraview
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
// BioDynaMo
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/string.h"
#include "core/util/thread_info.h"

namespace bdm {

// -----------------------------------------------------------------------------
void FixPvtu(const std::string& filename, const std::string& file_prefix,
             uint64_t pieces) {
  // read whole pvtu file into buffer
  std::ifstream ifs(filename);
  ifs.seekg(0, std::ios::end);
  size_t size = ifs.tellg();
  std::string buffer(size, ' ');
  ifs.seekg(0);
  ifs.read(&buffer[0], size);

  // create new file
  std::string find = Concat("<Piece Source=\"", file_prefix, "_0.vtu\"/>");
  std::stringstream new_file;
  auto pos = buffer.find(find);
  if (pos == std::string::npos) {
    // No agents in of this type in this timestep
    return;
  }
  new_file << buffer.substr(0, pos);
  for (uint64_t i = 0; i < pieces; ++i) {
    new_file << "<Piece Source=\"" << file_prefix << "_" << i << ".vtu\"/>\n";
  }
  new_file << buffer.substr(pos + find.size(), buffer.size());
  // write new file
  std::ofstream ofs(filename);
  ofs << new_file.str();
}

// -----------------------------------------------------------------------------
void ParallelVtuWriter::operator()(
    const std::string& folder, const std::string& file_prefix,
    const std::vector<vtkUnstructuredGrid*>& grids) const {
  auto* tinfo = ThreadInfo::GetInstance();
  auto* param = Simulation::GetActive()->GetParam();

#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < tinfo->GetMaxThreads(); ++i) {
    if (i == 0) {
      vtkNew<vtkXMLPUnstructuredGridWriter> pvtu_writer;
      auto filename = Concat(folder, "/", file_prefix, ".pvtu");
      pvtu_writer->SetFileName(filename.c_str());
      auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
      pvtu_writer->SetInputData(grids[0]);
      pvtu_writer->SetDataModeToBinary();
      pvtu_writer->SetEncodeAppendedData(false);
      if (!param->visualization_compress_pv_files) {
        pvtu_writer->SetCompressorTypeToNone();
      }
      pvtu_writer->Write();

      FixPvtu(filename, file_prefix, max_threads);
    } else {
      vtkNew<vtkXMLUnstructuredGridWriter> vtu_writer;
      auto filename = Concat(folder, "/", file_prefix, "_", i, ".vtu");
      vtu_writer->SetFileName(filename.c_str());
      vtu_writer->SetInputData(grids[i]);
      vtu_writer->SetDataModeToBinary();
      vtu_writer->SetEncodeAppendedData(false);
      if (!param->visualization_compress_pv_files) {
        vtu_writer->SetCompressorTypeToNone();
      }
      vtu_writer->Write();
    }
  }
}

}  // namespace bdm
