// // -----------------------------------------------------------------------------
// //
// // Copyright (C) The BioDynaMo Project.
// // All Rights Reserved.
// //
// // Licensed under the Apache License, Version 2.0 (the "License");
// // you may not use this file except in compliance with the License.
// //
// // See the LICENSE file distributed with this work for details.
// // See the NOTICE file distributed with this work for additional information
// // regarding copyright ownership.
// //
// // -----------------------------------------------------------------------------
//
// #ifndef CORE_THERMAL_GRID_H_
// #define CORE_THERMAL_GRID_H_
//
// #include <assert.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <algorithm>
// #include <array>
// #include <cmath>
// #include <functional>
// #include <iostream>
// #include <string>
// #include <vector>
// #include "core/util/root.h"
//
// #include "core/container/parallel_resize_vector.h"
// #include "core/param/param.h"
// #include "core/util/log.h"
// #include "core/util/math.h"
// #include "core/diffusion_grid.h"
//
//
// namespace bdm {
//
//   class ThermalGrid : public DiffusionGrid {
//
//   public :
//       /* Has a substance id of 2^20 as this is the max value of an int that can be assigned to uin64_t and will prevent conflict with the diffusion grid */
//   ThermalGrid( double thermal_diffusivity, int resolution,size_t substance_id):DiffusionGrid( substance_id, "Temperature" , thermal_diffusivity, 0 ,resolution)
//   {}
//
//   /* Call Temperature simply calls to GetConcentration */
//   double GetTemperature(const std::array<double, 3>& position){ return DiffusionGrid::GetConcentration(position);}
//
//   /* Call for increasing temperature through diffusion grid */
//   void IncreaseTemperatureBy(const std::array<double, 3>& position,double amount)
//   { DiffusionGrid::IncreaseConcentrationBy(position, amount);}
//
//   /* Get all Temperature data in our grid */
//   double* GetAllTemperatures(){return DiffusionGrid::GetAllConcentrations();}
//
//   void SetTemperatureThreshold(double t){DiffusionGrid::SetConcentrationThreshold(t);}
//
//   /* Simillar to above, as thermal diffusivity is being used as our dc we just need to call dc from the diffusion grid. */
//   std::array<double, 7>& GetThermalCoefficients() { return DiffusionGrid :: GetDiffusionCoefficients(); }
//
//   void SetThermalDiffusivity(double thermal_diffusivity) { thermal_diffusivity_ = thermal_diffusivity; }
//
//   std::array<double, 7>& GetDiffusionCoefficients(){return DiffusionGrid::GetDiffusionCoefficients();}
//
//
//
//   private :
//   /* Our thermal diffusivity to be used instead of diffusion coefficient */
//   double thermal_diffusivity_ = 0.1;
//
//
//   };
// } // namespace bdm
//
// #endif // CORE_THERMAL_GRID_H_

