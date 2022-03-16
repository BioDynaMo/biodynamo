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

#include "core/visualization/paraview/parallel_vti_writer.h"
// Paraview
#include <vtkDataArray.h>
#include <vtkDataCompressor.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkXMLPImageDataWriter.h>
// BioDynaMo
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/string.h"

namespace bdm {

// -----------------------------------------------------------------------------
VtiWriter::VtiWriter() = default;

// -----------------------------------------------------------------------------
void VtiWriter::SetWholeExtent(const int* whole_extent) {
  whole_extent_ = whole_extent;
}

// -----------------------------------------------------------------------------
void VtiWriter::WritePrimaryElementAttributes(std::ostream& os,
                                              vtkIndent indent) {
  this->WriteVectorAttribute("WholeExtent", 6, const_cast<int*>(whole_extent_));
  vtkImageData* input = this->GetInput();
  this->WriteVectorAttribute("Origin", 3, input->GetOrigin());
  this->WriteVectorAttribute("Spacing", 3, input->GetSpacing());
}

// -----------------------------------------------------------------------------
vtkStandardNewMacro(VtiWriter);

// -----------------------------------------------------------------------------
void PvtiWriter::Write(const std::string& folder,
                       const std::string& file_prefix,
                       const std::array<int, 6>& whole_extent,
                       const std::vector<std::array<int, 6>>& piece_extents,
                       vtkImageData* img, VtiWriter* vti) {
  auto* origin = img->GetOrigin();
  auto* spacing = img->GetSpacing();
  auto endianess_str = vti->GetByteOrder() == vtkXMLWriter::LittleEndian
                           ? "LittleEndian"
                           : "BigEndian";
  auto header_type_str =
      vti->GetHeaderType() == vtkXMLWriter::UInt32 ? "UInt32" : "UInt64";
  auto* compressor = vti->GetCompressor();
  std::string compressor_str =
      compressor != nullptr ? compressor->GetClassName() : "";
  // vti->GetDataSetMajorVersion() and vti->GetDataSetMinorVersion() are
  // protected
  auto version_str = "0.1";

  auto filename = Concat(folder, "/", file_prefix, ".pvti");
  std::ofstream ofs(filename);
  // header
  ofs << "<?xml version=\"1.0\"?>\n"
      << "<VTKFile type=\"PImageData\" "
      << "version=\"" << version_str << "\" "
      << "byte_order=\"" << endianess_str << "\" "
      << "header_type=\"" << header_type_str << "\" "
      << "compressor=\"" << compressor_str << "\">\n";
  ofs << "<PImageData WholeExtent=\"" << ArrayToString(whole_extent.data(), 6)
      << "\" GhostLevel=\"0\" Origin=\"" << ArrayToString(origin, 3)
      << "\" Spacing=\"" << ArrayToString(spacing, 3) << "\">\n";
  ofs << "  <PPointData>\n";

  // data arrays
  auto* pd = img->GetPointData();
  for (int i = 0; i < pd->GetNumberOfArrays(); ++i) {
    auto name = pd->GetArray(i)->GetName();
    auto components = pd->GetArray(i)->GetNumberOfComponents();
    ofs << "      <PDataArray type=\"Float64\" Name=\"" << name
        << "\" NumberOfComponents=\"" << components << "\"/>\n";
  }
  ofs << "  </PPointData>\n";

  // pieces
  int counter = 0;
  for (auto& e : piece_extents) {
    auto vti_filename = Concat(file_prefix, "_", counter++, ".vti");
    ofs << "    <Piece Extent=\"" << ArrayToString(e.data(), 6)
        << "\" Source=\"" << vti_filename << "\"/>\n";
  }

  ofs << "  </PImageData>\n";
  ofs << "</VTKFile>\n";
}

// -----------------------------------------------------------------------------
void ParallelVtiWriter::operator()(
    const std::string& folder, const std::string& file_prefix,
    const std::vector<vtkImageData*>& images, uint64_t num_pieces,
    const std::array<int, 6>& whole_extent,
    const std::vector<std::array<int, 6>>& piece_extents) const {
  auto* param = Simulation::GetActive()->GetParam();

#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < num_pieces; ++i) {
    auto vti_filename = Concat(folder, "/", file_prefix, "_", i, ".vti");
    vtkNew<VtiWriter> vti;
    vti->SetFileName(vti_filename.c_str());
    vti->SetInputData(images[i]);
    vti->SetWholeExtent(whole_extent.data());
    vti->SetDataModeToBinary();
    vti->SetEncodeAppendedData(false);
    if (!param->visualization_compress_pv_files) {
      vti->SetCompressorTypeToNone();
    }
    vti->Write();

    if (i == 0) {
      PvtiWriter pvti;
      pvti.Write(folder, file_prefix, whole_extent, piece_extents, images[0],
                 vti);
    }
  }
}

}  // namespace bdm
