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

// Based on default Kitware glyph

#ifndef PARAVIEW_PLUGIN_BDM_GLYPH_BDMGLYPH_H_
#define PARAVIEW_PLUGIN_BDM_GLYPH_BDMGLYPH_H_

#include "vtkGlyph3D.h"

#define VTK_SCALE_BY_NORMAL 4

class VTK_EXPORT BDMGlyph : public vtkGlyph3D {
 public:
  static BDMGlyph* New();
  vtkTypeMacro(BDMGlyph, vtkGlyph3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

 protected:
  BDMGlyph();
  ~BDMGlyph();

  //@{
  /**
   * Method called in RequestData() to do the actual data processing. This will
   * glyph the \c input, filling up the \c output based on the filter
   * parameters.
   */
  bool Execute(vtkDataSet* input, vtkInformationVector* sourceVector,
               vtkPolyData* output) override;
  bool Execute(vtkDataSet* input, vtkInformationVector* sourceVector,
               vtkPolyData* output, vtkDataArray* inSScalars,
               vtkDataArray* inVectors) override;

 private:
  BDMGlyph(const BDMGlyph&) = delete;
  void operator=(const BDMGlyph&) = delete;
};

#endif  // PARAVIEW_PLUGIN_BDM_GLYPH_BDMGLYPH_H_
