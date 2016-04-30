#ifndef PHYSICS_INTRACELLULAR_SUBSTANCE_H_
#define PHYSICS_INTRACELLULAR_SUBSTANCE_H_

#include <string>
#include <memory>

#include "param.h"
#include "physics/substance.h"

namespace cx3d {
namespace physics {

class IntracellularSubstance : public Substance {
 public:
  IntracellularSubstance();

  static std::shared_ptr<IntracellularSubstance> create() {
    return std::shared_ptr<IntracellularSubstance>(new IntracellularSubstance());
  }

  static std::shared_ptr<IntracellularSubstance> create(const std::shared_ptr<IntracellularSubstance>& other) {
    return std::shared_ptr<IntracellularSubstance>(new IntracellularSubstance(*other));
  }

  static std::shared_ptr<IntracellularSubstance> create(const std::string& id, double diffusion_constant,
                                           double degradation_constant) {
    return std::shared_ptr<IntracellularSubstance>(new IntracellularSubstance(id, diffusion_constant, degradation_constant));
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /**
   * Distribute IntracellularSubstance concentration at division and update quantity.
   * @param new_is
   */
  virtual void distributeConcentrationOnDivision(std::shared_ptr<IntracellularSubstance>& new_is);

  /**
   * Degradation of the <code>IntracellularSubstance</code>.
   */
  virtual void degrade();

  /**
   * Returns the degree of asymmetric distribution during cell division.
   * 0 represents complete symmetrical division, 1 represents complete asymmetric division.
   */
  virtual double getAsymmetryConstant() const;

  /**
   * Sets the degree of asymmetric distribution during cell division.
   * 0 represents complete symmetrical division, 1 represents complete asymmetric division.
   * The sign + or - is used to distinguish between one daughter (mother cell) and the other
   * (new cell).
   */
  virtual void setAsymmetryConstant(double asymmetry_constant);

  /**
   * If true, the Substance can be detected from outside of the PhysicalObject
   * (equivalent to an membrane bound substance).
   */
  virtual bool isVisibleFromOutside() const;

  /**
   * If true, the Substance can be detected from outside of the PhysicalObject
   * (equivalent to an membrane bound substance).
   */
  virtual void setVisibleFromOutside(bool visible_from_outside);

  /**
   * If true, the volume is taken into account for computing the concentration,
   * otherwise a virtual volume corresponding to the length of the physical object
   * (with virtual radius 1) is used.
   */
  virtual bool isVolumeDependant() const;

  /**
   * If true, the volume is taken into account for computing the concentration,
   * otherwise a virtual volume corresponding to the length of the physical object
   * (with virtual radius 1) is used.
   */
  virtual void setVolumeDependant(bool volume_dependant);

  virtual std::shared_ptr<Substance> getCopy() const override;

  bool equalTo(const std::shared_ptr<IntracellularSubstance>& other){
    return this == other.get();
  }

 protected:
  IntracellularSubstance(const std::string& id, double diffusion_constant, double degradation_constant);

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
  IntracellularSubstance(const IntracellularSubstance& other);
  IntracellularSubstance& operator=(const IntracellularSubstance&) = delete;
};

}  // namespace physics
}  // namespace cx3d

#endif // PHYSICS_INTRACELLULAR_SUBSTANCE_H_
