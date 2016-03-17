#ifndef PARAM_H_
#define PARAM_H_

#include "color.h"

namespace cx3d {

class Param {
 public:
  /** Time between two simulation step, in hours. */
  static constexpr double kSimulationTimeStep = 0.01;

  // Diffusion (saving time by not running diffusion for too small differences)

  /** If concentration of a substance smaller than this, it is not diffused */
  static constexpr double kMinimalConcentrationForExtracellularDiffusion = 1e-5;

  /** If absolute concentration difference is smaller than that, there is no diffusion*/
  static constexpr double kMinimalDifferenceConcentrationForExtracacellularDiffusion = 1e-5;
  /** If ratio (absolute concentration difference)/concentration is smaller than that, no diffusion.*/
  static constexpr double kMinimalDCOverCForExtracellularDiffusion = 1e-3;

  /** If concentration of a substance smaller than this, it is not diffused */
  static constexpr double kMinimalConcentrationForIntracellularDiffusion  = 1e-10;
    /** If absolute concentration difference is smaller than that, there is no diffusion*/
  static constexpr double kMinimalDifferenceConcentrationForIntracellularDiffusion = 1e-7;
    /** If ration (absolute concentration difference)/concentration is smaller than that, no diffusion.*/
  static constexpr double kMinimalDCOverCForIntracellularDiffusion = 1e-4;

  // some colors, that we define, because we find them.. well beautiful.
  static constexpr Color kViolet = Color(0xFFD41F);
};

}  // namespace cx3d

#endif  // PARAM_H_
