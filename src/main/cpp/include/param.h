#ifndef PARAM_H_
#define PARAM_H_

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
};

#endif  // PARAM_H_
