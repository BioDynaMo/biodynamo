#ifndef LOCAL_BIOLOGY_SOMA_ELEMENT_H_
#define LOCAL_BIOLOGY_SOMA_ELEMENT_H_

#include <exception>
#include <string>
#include <vector>

#include "local_biology/cell_element.h"
#include "physics/physical_sphere.h"

namespace bdm {

namespace local_biology {

class NeuriteElement;

using physics::PhysicalObject;
using physics::PhysicalSphere;

/**
 * This class contains the description of the biological properties of a soma (if it contains
 * instances of <code>LocalBiologyModule</code>. It is asociated with a <code>PhysicalSphere</code>.
 */
class SomaElement : public CellElement {
 public:
  using UPtr = std::unique_ptr<SomaElement>;

  SomaElement();

  virtual ~SomaElement();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  UPtr divide(double volume_ratio, double phi, double theta);

  void run();

  NeuriteElement* extendNewNeurite();

  NeuriteElement* extendNewNeurite(double diameter);

  NeuriteElement* extendNewNeurite(const std::array<double, 3>& direction);

  NeuriteElement* extendNewNeurite(double diameter, const std::array<double, 3>& direction);

  /**
   * Extends a new neurites
   * @param diameter the diameter of the new neurite
   * @param phi the angle from the zAxis
   * @param theta the angle from the xAxis around the zAxis
   * @return
   */
  NeuriteElement* extendNewNeurite(double diameter, double phi, double theta);

  //todo implement makeSomaticSpines

  PhysicalObject* getPhysical() const override;

  void setPhysical(PhysicalObject::UPtr po) override;

  PhysicalSphere* getPhysicalSphere() const;

  void setPhysicalSphere(PhysicalSphere::UPtr po);

  std::vector<NeuriteElement*> getNeuriteList() const;

  bool isANeuriteElement() const override;

  bool isASomaElement() const override;

 private:
  SomaElement(const SomaElement&) = delete;
  SomaElement& operator=(const SomaElement&) = delete;

  PhysicalSphere::UPtr physical_;
};

}  // namespace local_biology
}  // namespace bdm

#endif  // LOCAL_BIOLOGY_SOMA_ELEMENT_H_
