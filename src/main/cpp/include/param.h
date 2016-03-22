#ifndef PARAM_H_
#define PARAM_H_

#include "color.h"

namespace cx3d {

class Param {
 public:
  /** Time between two simulation step, in hours. */
  static constexpr double kSimulationTimeStep = 0.01;
  /** Maximum jump that a point mass can do in one time step. Useful to stabilize the simulation*/
  static constexpr double kSimulationMaximalDisplacement = 3.0;

  // Diffusion (saving time by not running diffusion for too small differences)

  /** If concentration of a substance smaller than this, it is not diffused */
  static constexpr double kMinimalConcentrationForExtracellularDiffusion = 1e-5;

  /** If absolute concentration difference is smaller than that, there is no diffusion*/
  static constexpr double kMinimalDifferenceConcentrationForExtracacellularDiffusion = 1e-5;
  /** If ratio (absolute concentration difference)/concentration is smaller than that, no diffusion.*/
  static constexpr double kMinimalDCOverCForExtracellularDiffusion = 1e-3;

  /** If concentration of a substance smaller than this, it is not diffused */
  static constexpr double kMinimalConcentrationForIntracellularDiffusion = 1e-10;
  /** If absolute concentration difference is smaller than that, there is no diffusion*/
  static constexpr double kMinimalDifferenceConcentrationForIntracellularDiffusion = 1e-7;
  /** If ration (absolute concentration difference)/concentration is smaller than that, no diffusion.*/
  static constexpr double kMinimalDCOverCForIntracellularDiffusion = 1e-4;

  // Neurites
  /** Initial value of the restingLength before any specification.*/
  static constexpr double kNeuriteDefaultActualLength = 1.0;
  /** Diameter of an unspecified (= axon/dendrite) neurite when extends from the somaElement */
  static constexpr double kNeuriteDefaultDiameter = 1.0;
  static constexpr double kNeuriteMinimalBifurcationLength = 0;
  /** Spring constant*/
  static constexpr double kNeuriteDefaultSpringConstant = 10;  // 10;
  /** Threshold the force acting on a neurite has to reach before a move is made ( = static friction).*/
  static constexpr double kNeuriteDefaultAdherence = 0.1;
  /** Rest to the movement ( = kinetic friction).*/
  static constexpr double kNeuriteDefaultMass = 1;

  static constexpr double kNeuriteDefaultTension = 0.0;

  // Somata
      /** CAUTION: not the radius but the diameter*/
  static constexpr double kSphereDefaultDiameter = 20;
    /** Threshold the force acting on a somaElement has to reach before a move is made ( = static friction).*/
  static constexpr double kSphereDefaultAdherence = 0.4;
    /** Restistance to the movement ( = kinetic friction).*/
  static constexpr double kSphereDefaultMass = 1;

  // some colors, that we define, because we find them.. well beautiful.
  static constexpr Color kViolet = Color(0xFFFFD41F);
};

}  // namespace cx3d

#endif  // PARAM_H_
