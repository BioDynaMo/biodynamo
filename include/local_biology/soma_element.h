#ifndef LOCAL_BIOLOGY_SOMA_ELEMENT_H_
#define LOCAL_BIOLOGY_SOMA_ELEMENT_H_

#include <exception>
#include <string>

#include "local_biology/cell_element.h"

namespace cx3d {

namespace physics {
class PhysicalObject;
class PhysicalSphere;
}  // namespace physics

namespace local_biology {

class NeuriteElement;

class SomaElement : public CellElement {
 public:
  using UPtr = std::unique_ptr<SomaElement>;

  SomaElement();  // fixme protected after porting is complete

  virtual ~SomaElement();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual UPtr divide(double volume_ratio, double phi, double theta);

  virtual void run();

  virtual NeuriteElement* extendNewNeurite();

  virtual NeuriteElement* extendNewNeurite(double diameter);

  virtual NeuriteElement* extendNewNeurite(const std::array<double, 3>& direction);

  virtual NeuriteElement* extendNewNeurite(double diameter, const std::array<double, 3>& direction);

  /**
   * Extends a new neurites
   * @param diameter the diameter of the new neurite
   * @param phi the angle from the zAxis
   * @param theta the angle from the xAxis around the zAxis
   * @return
   */
  virtual NeuriteElement* extendNewNeurite(double diameter, double phi, double theta);

  //todo implement makeSomaticSpines

  virtual std::shared_ptr<physics::PhysicalObject> getPhysical() const override;

  virtual void setPhysical(const std::shared_ptr<physics::PhysicalObject>& po) override;

  virtual std::shared_ptr<physics::PhysicalSphere> getPhysicalSphere() const;

  virtual void setPhysicalSphere(const std::shared_ptr<physics::PhysicalSphere>& po);

  virtual std::list<NeuriteElement*> getNeuriteList() const;

  virtual bool isANeuriteElement() const override;

  virtual bool isASomaElement() const override;

 private:
  SomaElement(const SomaElement&) = delete;
  SomaElement& operator=(const SomaElement&) = delete;

  std::shared_ptr<physics::PhysicalSphere> physical_;
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_SOMA_ELEMENT_H_
