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

#ifndef CORE_THERMAL_GRID_H_
#define CORE_THERMAL_GRID_H_

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include "core/util/root.h"

#include "core/container/math_array.h"
#include "core/container/parallel_resize_vector.h"
#include "core/diffusion_grid.h"
#include "core/param/param.h"
#include "core/util/log.h"
#include "core/util/math.h"

namespace bdm {

  // The thermal grid derives a considerable amount of code from the Diffusion grid to ensure consistency across multiple grids.

class ThermalGrid : public DiffusionGrid {
 public:
  /* Has a substance id of 2^20 as this is the max value of an int that can be
   * assigned to uin64_t and will prevent conflict with the diffusion grid */
  ThermalGrid(double thermal_diffusivity, int resolution, size_t substance_id)
      : DiffusionGrid(substance_id, "Temperature", thermal_diffusivity, 0,
                      resolution) {}

  void Initialize(const std::array<int32_t, 6>& grid_dimensions){
   return DiffusionGrid::Initialize(grid_dimensions);
  }

  void ParametersCheck(){
    return DiffusionGrid::ParametersCheck();
  }

  void RunInitializers() {
    return DiffusionGrid::RunInitializers();
  }

  void Update(const std::array<int32_t, 2>& threshold_dimensions) {
    return DiffusionGrid::Update(threshold_dimensions);
  }

  void CopyOldData(const ParallelResizeVector<double>& old_c1,
                   const ParallelResizeVector<double>& old_gradients,
                   const std::array<size_t, 3>& old_num_boxes_axis) {
   return DiffusionGrid::CopyOldData(old_c1,old_gradients,old_num_boxes_axis);
  }

  void DiffuseWithLeakingEdge(){
    return DiffusionGrid::DiffuseWithLeakingEdge();
  }

  void DiffuseWithClosedEdge() {
    return DiffusionGrid::DiffuseWithClosedEdge();
  }

  void DiffuseEuler(){
    return DiffusionGrid::DiffuseEuler();
  }

  void DiffuseEulerLeakingEdge() {
    return DiffusionGrid::DiffuseEulerLeakingEdge();
  }

  void CalculateGradient() {
    return DiffusionGrid::CalculateGradient();
  }

  /* Call Temperature simply calls to GetConcentration */
  double GetTemperature(const Double3& position) {
    return DiffusionGrid::GetConcentration(position);
  }

  /* Call for increasing temperature via internal node heat generation through diffusion grid */
  void IncreaseTemperatureBy(const Double3& position, double generation, double conductivity) {
    /* Here generation is internal heat generation in W/m^(3) and conductivity is the thermal conductivity in W/(m * K)
    or W/(m x C) depedning on your own defining of temperature units.
    + generation acts as a source, - acts as a sink*/

    double amount = (generation/conductivity);
      return DiffusionGrid::IncreaseConcentrationBy(position, amount);
  }
  double GetTemperature(const Double3& position) const {
    return DiffusionGrid::GetConcentration(position);
  }

   std::array<uint32_t, 3> GetBoxCoordinates(const Double3& position) const {
     return DiffusionGrid::GetBoxCoordinates(position);
   }

   size_t GetBoxIndex(const std::array<uint32_t, 3>& box_coord) const {
     return DiffusionGrid::GetBoxIndex(box_coord);
   }

   size_t GetBoxIndex(const Double3& position) const {
     return DiffusionGrid::GetBoxIndex(position);
   }

   void SetTemperatureThreshold(double t) {
     return DiffusionGrid::SetConcentrationThreshold(t);
   }

   double GetTemperatureThreshold() const {
     return DiffusionGrid::GetConcentrationThreshold();
   }

  void SetTemperatureLowerThreshold(double l) {
    return DiffusionGrid::SetConcentrationLowerThreshold(l);
  }

  double GetTemperatureLowerThreshold() {
    return DiffusionGrid::GetConcentrationLowerThreshold();
  }

  /* Get all Temperature data in our grid */
  const double* GetAllTemperatures() {
    return DiffusionGrid::GetAllConcentrations();
  }

  const double* GetAllGradients() const {
    return DiffusionGrid::GetAllGradients();
  }

  const std::array<size_t, 3>& GetNumBoxesArray() const {
    return DiffusionGrid::GetNumBoxesArray();
  }

  size_t GetNumBoxes() const {
    return DiffusionGrid::GetNumBoxes();
  }

  double GetBoxLength() const {
    return DiffusionGrid::GetBoxLength();
  }

  int GetSubstanceId() const {
    return DiffusionGrid::GetSubstanceId();
  }

  const int32_t* GetDimensionsPtr() const {
    return DiffusionGrid::GetDimensionsPtr();
  }

  const std::array<int32_t, 6>& GetDimensions() const {
    return DiffusionGrid::GetDimensions();
  }

   bool IsInitialized() const {
     return DiffusionGrid::IsInitialized();
   }

  int GetResolution() const {
    return DiffusionGrid::GetResolution();
  }

  double GetBoxVolume() const {
    return DiffusionGrid::GetBoxVolume();
  }

  /* Simillar to above, as thermal diffusivity is being used as our dc we just
   * need to call dc from the diffusion grid. */
  const std::array<double, 7>& GetThermalCoefficients() {
    return DiffusionGrid::GetDiffusionCoefficients();
  }

  void SetThermalDiffusivity(double thermal_diffusivity) {
    thermal_diffusivity_ = thermal_diffusivity;
  }

  const std::array<double, 7>& GetDiffusionCoefficients() {
    return DiffusionGrid::GetDiffusionCoefficients();
  }



 private:
  /* Our thermal diffusivity to be used instead of diffusion coefficient, this is
   simply a default value and can be altered simillarly to the diffusion coefficient*/

  double thermal_diffusivity_ = 0.1;
};
}  // namespace bdm

#endif  // CORE_THERMAL_GRID_H_

   return DiffusionGrid::Initialize(grid_dimensions);
  }

  void ParametersCheck(){
    return DiffusionGrid::ParametersCheck();
  }

  void RunInitializers() {
    return DiffusionGrid::RunInitializers();
  }

  void Update(const std::array<int32_t, 2>& threshold_dimensions) {
    return DiffusionGrid::Update(threshold_dimensions);
  }

  void CopyOldData(const ParallelResizeVector<double>& old_c1,
                   const ParallelResizeVector<double>& old_gradients,
                   const std::array<size_t, 3>& old_num_boxes_axis) {
   return DiffusionGrid::CopyOldData(old_c1,old_gradients,old_num_boxes_axis);
  }

  void DiffuseWithLeakingEdge(){
    return DiffusionGrid::DiffuseWithLeakingEdge();
  }

  void DiffuseWithClosedEdge() {
    return DiffusionGrid::DiffuseWithClosedEdge();
  }

  void DiffuseEuler(){
    return DiffusionGrid::DiffuseEuler();
  }

  void DiffuseEulerLeakingEdge() {
    return DiffusionGrid::DiffuseEulerLeakingEdge();
  }

  void CalculateGradient() {
    return DiffusionGrid::CalculateGradient();
  }

  /* Call Temperature simply calls to GetConcentration */
  double GetTemperature(const Double3& position) {
    return DiffusionGrid::GetConcentration(position);
  }

  /* Call for increasing temperature through diffusion grid */
  void IncreaseTemperatureBy(const Double3& position, double amount) {
    return DiffusionGrid::IncreaseConcentrationBy(position, amount);
  }

  double GetTemperature(const Double3& position) const {
    return DiffusionGrid::GetConcentration(position);
  }

   std::array<uint32_t, 3> GetBoxCoordinates(const Double3& position) const {
     return DiffusionGrid::GetBoxCoordinates(position);
   }

   size_t GetBoxIndex(const std::array<uint32_t, 3>& box_coord) const {
     return DiffusionGrid::GetBoxIndex(box_coord);
   }

   size_t GetBoxIndex(const Double3& position) const {
     return DiffusionGrid::GetBoxIndex(position);
   }

   void SetTemperatureThreshold(double t) {
     return DiffusionGrid::SetConcentrationThreshold(t);
   }

   double GetTemperatureThreshold() const {
     return DiffusionGrid::GetConcentrationThreshold();
   }

  void SetTemperatureLowerThreshold(double l) {
    return DiffusionGrid::SetConcentrationLowerThreshold(l);
  }

  double GetTemperatureLowerThreshold() {
    return DiffusionGrid::GetConcentrationLowerThreshold();
  }

  /* Get all Temperature data in our grid */
  const double* GetAllTemperatures() {
    return DiffusionGrid::GetAllConcentrations();
  }

  const double* GetAllGradients() const {
    return DiffusionGrid::GetAllGradients();
  }

  const std::array<size_t, 3>& GetNumBoxesArray() const {
    return DiffusionGrid::GetNumBoxesArray();
  }

  size_t GetNumBoxes() const {
    return DiffusionGrid::GetNumBoxes();
  }

  double GetBoxLength() const {
    return DiffusionGrid::GetBoxLength();
  }

  int GetSubstanceId() const {
    return DiffusionGrid::GetSubstanceId();
  }

  const int32_t* GetDimensionsPtr() const {
    return DiffusionGrid::GetDimensionsPtr();
  }

  const std::array<int32_t, 6>& GetDimensions() const {
    return DiffusionGrid::GetDimensions();
  }

   bool IsInitialized() const {
     return DiffusionGrid::IsInitialized();
   }

  int GetResolution() const {
    return DiffusionGrid::GetResolution();
  }

  double GetBoxVolume() const {
    return DiffusionGrid::GetBoxVolume();
  }

  /* Simillar to above, as thermal diffusivity is being used as our dc we just
   * need to call dc from the diffusion grid. */
  const std::array<double, 7>& GetThermalCoefficients() {
    return DiffusionGrid::GetDiffusionCoefficients();
  }

  void SetThermalDiffusivity(double thermal_diffusivity) {
    thermal_diffusivity_ = thermal_diffusivity;
  }

  const std::array<double, 7>& GetDiffusionCoefficients() {
    return DiffusionGrid::GetDiffusionCoefficients();
  }

 private:
  /* Our thermal diffusivity to be used instead of diffusion coefficient, this is
   simply a default value and can be altered simillarly to the diffusion coefficient*/

  double thermal_diffusivity_ = 0.1;
};
}  // namespace bdm

#endif  // CORE_THERMAL_GRID_H_
