#ifndef PHYSICS_INTRACELLULAR_SUBSTANCE_H_
#define PHYSICS_INTRACELLULAR_SUBSTANCE_H_

#include <string>
#include <memory>

#include "param.h"
#include "physics/substance.h"

namespace bdm {
namespace physics {

/**
 * Instances of this class represent the intracellular and surface (membrane bound)
 * Substances. The visibility from outside (fact that they are expressed on the surface)
 * is specified by the appropriate getter and setter.
 */
class IntracellularSubstance : public Substance {
 public:
  using UPtr = std::unique_ptr<IntracellularSubstance>;

  IntracellularSubstance(TRootIOCtor*) { }  // only used for ROOT I/O
  
  IntracellularSubstance();

  IntracellularSubstance(const IntracellularSubstance& other);

  IntracellularSubstance(const std::string& id, double diffusion_constant, double degradation_constant);

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /**
   * Distribute IntracellularSubstance concentration at division and update quantity.
   * @param new_is
   */
  void distributeConcentrationOnDivision(IntracellularSubstance* new_is);

  /**
   * Degradation of the <code>IntracellularSubstance</code>.
   */
  void degrade();

  /**
   * Returns the degree of asymmetric distribution during cell division.
   * 0 represents complete symmetrical division, 1 represents complete asymmetric division.
   */
  double getAsymmetryConstant() const;

  /**
   * Sets the degree of asymmetric distribution during cell division.
   * 0 represents complete symmetrical division, 1 represents complete asymmetric division.
   * The sign + or - is used to distinguish between one daughter (mother cell) and the other
   * (new cell).
   */
  void setAsymmetryConstant(double asymmetry_constant);

  /**
   * If true, the Substance can be detected from outside of the PhysicalObject
   * (equivalent to an membrane bound substance).
   */
  bool isVisibleFromOutside() const;

  /**
   * If true, the Substance can be detected from outside of the PhysicalObject
   * (equivalent to an membrane bound substance).
   */
  void setVisibleFromOutside(bool visible_from_outside);

  /**
   * If true, the volume is taken into account for computing the concentration,
   * otherwise a virtual volume corresponding to the length of the physical object
   * (with virtual radius 1) is used.
   */
  bool isVolumeDependant() const;

  /**
   * If true, the volume is taken into account for computing the concentration,
   * otherwise a virtual volume corresponding to the length of the physical object
   * (with virtual radius 1) is used.
   */
  void setVolumeDependant(bool volume_dependant);

  virtual Substance::UPtr getCopy() const override;

 protected:
  /**
   * Degree of asymmetric distribution during cell division.
   * 0 represents complete symmetrical division, 1 represents complete asymmetric division.
   */
  double asymmetry_constant_;

  /**
   * If true, the Substance can be detected from outside of the PhysicalObject
   * (equivalent to an membrane bound substance).
   */
  bool visible_from_outside_;

  /**
   * If true, the volume is taken into account for computing the concentration,
   * otherwise quantity and volume are considered as equivalent (effective volume = 1).
   */
  bool volume_dependant_;

 private:
  /**
   * Copies the physical properties of the IntracellularSubstance given as argument
   * (id, diffusionConstant, degradationConstant, color, visibleFromOutside, volumeDependant),
   * but : QUANTITY AND CONCENTRATION ARE NOT COPIED !!
   * @param other
   */
  IntracellularSubstance& operator=(const IntracellularSubstance&) = delete;

  ClassDefOverride(IntracellularSubstance, 0);
};

}  // namespace physics
}  // namespace bdm

#endif // PHYSICS_INTRACELLULAR_SUBSTANCE_H_
