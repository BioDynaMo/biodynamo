// // -----------------------------------------------------------------------------
// //
// // Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// // BioDynaMo collaboration. All Rights Reserved.
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

#ifndef MY_FUNCS_H_
#define MY_FUNCS_H_

#include <ctime>
#include <string>
#include <vector>

#include "core/container/math_array.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/random.h"


#include "core/substance_initializers.h"
#include "Math/DistFunc.h"
#include "core/diffusion/diffusion_grid.h"




// #ifndef CORE_MODEL_INITIALIZER_H_
// #define CORE_MODEL_INITIALIZER_H_
//
// #include <ctime>
// #include <string>
// #include <vector>
//
// #include "core/container/math_array.h"
// #include "core/diffusion/diffusion_grid.h"
// #include "core/resource_manager.h"
// #include "core/simulation.h"
// #include "core/util/random.h"
//
// class EulerGrid;
// class StencilGrid;
// class RungaKuttaGrid;
//
namespace bdm {

  template <typename Function>
  static void Grid2D(size_t agents_per_dim, double space,
    Function agent_builder) {
      #pragma omp parallel
      {
        auto* sim = Simulation::GetActive();
        auto* ctxt = sim->GetExecutionContext();

        #pragma omp for
        for (size_t x = 0; x < agents_per_dim; x++) {
          auto x_pos = x * space;
          for (size_t y = 0; y < agents_per_dim; y++) {
            auto y_pos = y * space;
            //for (size_t z = 0; z < agents_per_dim; z++) {
            auto* new_agent = agent_builder({x_pos, y_pos, 0.0});
            new_agent->SetMass(0.0001);
            ctxt->AddAgent(new_agent);
            //}
          }
        }
      }
    }
  }


//}
  enum Axis { kXAxis, kYAxis, kZAxis };


  /// An initializer that follows a Poisson (normal) distribution along one axis
  /// In contrast to the previous function, the location of the peak can be set
  /// The function ROOT::Math::poisson_pdfd(X, lambda) follows the normal
  /// probability density function:
  /// {e^( - lambda ) * lambda ^x )} / x!
  class PoissonBandAtPos {
  public:
    /// @brief      The constructor
    ///
    /// @param[in]  lambda The lambda of the Poisson distribution
    /// @param[in]  axis   The axis along which you want the Poisson distribution
    ///                    to be oriented to
    ///
    //PoissonBandAtPos(double lambda, uint8_t axis, Double3 position) {
    PoissonBandAtPos(double lambda, uint8_t axis, std::array<double, 3> position) {
      lambda_ = lambda;
      axis_ = axis;
      pos_x_ = position[0];
      pos_y_ = position[1];
      pos_z_ = position[2];
    }

    /// @brief      The model that we want to apply for substance initialization.
    ///             The operator is called for the entire space
    ///
    /// @param[in]  x     The x coordinate
    /// @param[in]  y     The y coordinate
    /// @param[in]  z     The z coordinate
    ///
    double operator()(double x, double y, double z) {
      switch (axis_) {
        case Axis::kXAxis:
        return ROOT::Math::poisson_pdf(x-pos_x_, lambda_);
        case Axis::kYAxis:
        return ROOT::Math::poisson_pdf(y-pos_y_, lambda_);
        case Axis::kZAxis:
        return ROOT::Math::poisson_pdf(z-pos_z_, lambda_);
        default:
        throw std::logic_error("You have chosen an non-existing axis!");
      }
    }

  private:
    double lambda_;
    uint8_t axis_;
    double pos_x_;
    double pos_y_;
    double pos_z_;
  };

//}  // namespace bdm

#endif  // CORE_MODEL_INITIALIZER_H_
