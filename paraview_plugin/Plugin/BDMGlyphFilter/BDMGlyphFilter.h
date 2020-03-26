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

// Based on default Kitware glyph

/**
 * @class   BDMGlyphFilter
 * @brief   extended API for vtkGlyph3DP for better control
 * over glyph placement.
 *
 *
 * BDMGlyphFilter extends vtkGlyph3DP for adding control over which points
 * are
 * glyphed using \c GlyphMode. Three modes are now provided:
 * \li ALL_POINTS: all points in the input dataset are glyphed. This same as
 * using
 * vtkGlyph3DP directly.
 *
 * \li EVERY_NTH_POINT: every n-th point in the input dataset when iterated
 * through the input points sequentially is glyphed. For composite datasets,
 * the counter resets every on block. In parallel, independent counter is used
 * on each rank. Use \c Stride to control now may points to skip.
 *
 * \li SPATIALLY_UNIFORM_DISTRIBUTION: points close to a randomly sampled
 * spatial
 * distribution of points are glyphed. \c Seed controls the seed point for the
 * random
 * number generator (vtkMinimalStandardRandomSequence). \c
 * MaximumNumberOfSamplePoints
 * can be used to limit the number of sample points used for random sampling.
 * This
 * doesn't not equal the number of points actually glyphed, since that depends
 * on
 * several factors. In parallel, this filter ensures that spatial bounds are
 * collected
 * across all ranks for generating identical sample points.
*/
#ifndef PARAVIEW_PLUGIN_BDM_GLYPH_BDMGLYPHFILTER_H_
#define PARAVIEW_PLUGIN_BDM_GLYPH_BDMGLYPHFILTER_H_

#include "BDMGlyph.h"
#include "vtkPVVTKExtensionsFiltersGeneralModule.h"  // needed for exports

class vtkMultiProcessController;

class VTKPVVTKEXTENSIONSFILTERSGENERAL_EXPORT BDMGlyphFilter : public BDMGlyph {
 public:
  enum GlyphModeType {
    ALL_POINTS,
    EVERY_NTH_POINT,
    SPATIALLY_UNIFORM_DISTRIBUTION
  };

  vtkTypeMacro(BDMGlyphFilter, BDMGlyph);
  void PrintSelf(ostream& os, vtkIndent indent) override;  // NOLINT
  static BDMGlyphFilter* New();

  //@{
  /**
   * Get/Set the vtkMultiProcessController to use for parallel processing.
   * By default, the vtkMultiProcessController::GetGlobalController() will be
   * used.
   */
  void SetController(vtkMultiProcessController* controller);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);
  //@}

  //@{
  /**
   * Set/Get the mode at which glyphs will be generated.
   */
  vtkSetClampMacro(GlyphMode, int, ALL_POINTS, SPATIALLY_UNIFORM_DISTRIBUTION);
  vtkGetMacro(GlyphMode, int);
  //@}

  //@{
  /**
   * Set/Get the stride at which to glyph the dataset.
   * Note, only applicable with EVERY_NTH_POINT GlyphMode.
   */
  vtkSetClampMacro(Stride, int, 1, VTK_INT_MAX);
  vtkGetMacro(Stride, int);
  //@}

  //@{
  /**
   * Set/Get Seed used for generating a spatially uniform distribution.
   */
  vtkSetMacro(Seed, int);
  vtkGetMacro(Seed, int);
  //@}

  //@{
  /**
   * Set/Get maximum number of sample points to use to sample the space when
   * GlyphMode is set to SPATIALLY_UNIFORM_DISTRIBUTION.
   */
  vtkSetClampMacro(MaximumNumberOfSamplePoints, int, 1, VTK_INT_MAX);
  vtkGetMacro(MaximumNumberOfSamplePoints, int);
  //@}

  //@{
  /**
   * Overridden to create output data of appropriate type.
   */
  virtual int ProcessRequest(vtkInformation*, vtkInformationVector**,
                             vtkInformationVector*) override;

 protected:
  BDMGlyphFilter();
  ~BDMGlyphFilter();
  //@}

  // Standard Pipeline methods
  virtual int RequestData(vtkInformation*, vtkInformationVector**,
                          vtkInformationVector*) override;
  virtual int RequestDataObject(vtkInformation*, vtkInformationVector**,
                                vtkInformationVector*);
  virtual int FillInputPortInformation(int, vtkInformation*) override;
  virtual int FillOutputPortInformation(int, vtkInformation*) override;

  /**
   * Returns 1 if point is to be glyped, otherwise returns 0.
   */
  virtual int IsPointVisible(vtkDataSet* ds, vtkIdType ptId) override;

  /**
   * Returns true if input Scalars and Vectors are compatible, otherwise returns
   * 0.
   */
  bool IsInputArrayToProcessValid(vtkDataSet* input);

  /**
   * Returns true if input Scalars and Vectors are cell attributes, otherwise
   * returns 0.
   */
  bool UseCellCenters(vtkDataSet* input);

  /**
   * Returns true if input scalars are used for glyphing.
   */
  bool NeedsScalars();

  /**
   * Returns true if input vectors are used for glyphing.
   */
  bool NeedsVectors();

  /**
   * Method called in RequestData() to do the actual data processing. This will
   * apply a Cell Centers before the Glyph. The \c input, filling up the \c
   * output
   * based on the filter parameters.
   */
  virtual bool ExecuteWithCellCenters(vtkDataSet* input,
                                      vtkInformationVector* sourceVector,
                                      vtkPolyData* output);

  int GlyphMode;
  int MaximumNumberOfSamplePoints;
  int Seed;
  int Stride;
  vtkMultiProcessController* Controller;

 private:
  BDMGlyphFilter(const BDMGlyphFilter&) = delete;
  void operator=(const BDMGlyphFilter&) = delete;

  class vtkInternals;
  vtkInternals* Internals;
};

#endif  // PARAVIEW_PLUGIN_BDM_GLYPH_BDMGLYPHFILTER_H_
