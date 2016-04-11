#ifndef LOCAL_BIOLOGY_SOMA_ELEMENT_H_
#define LOCAL_BIOLOGY_SOMA_ELEMENT_H_

#include <exception>
#include <string>

#include "cell_element.h"

namespace cx3d {

namespace physics {
class PhysicalObject;
class PhysicalSphere;
}  // namespace physics

namespace local_biology {

class NeuriteElement;

class SomaElement : public CellElement {
 public:
  static std::shared_ptr<SomaElement> create() {
    std::shared_ptr<SomaElement> soma { new SomaElement() };
    soma->init();
    return soma;
  }

  SomaElement();  // fixme protected after porting is complete

  virtual ~SomaElement() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual std::shared_ptr<SomaElement> divide(double volume_ratio, double phi, double theta);

  virtual void run();

  virtual std::shared_ptr<NeuriteElement> extendNewNeurite();

  virtual std::shared_ptr<NeuriteElement> extendNewNeurite(double diameter);

  virtual std::shared_ptr<NeuriteElement> extendNewNeurite(const std::array<double, 3>& direction);

  virtual std::shared_ptr<NeuriteElement> extendNewNeurite(double diameter, const std::array<double, 3>& direction);

  /**
   * Extends a new neurites
   * @param diameter the diameter of the new neurite
   * @param phi the angle from the zAxis
   * @param theta the angle from the xAxis around the zAxis
   * @return
   */
  virtual std::shared_ptr<NeuriteElement> extendNewNeurite(double diameter, double phi, double theta);

  virtual std::shared_ptr<physics::PhysicalObject> getPhysical() const override;

  virtual void setPhysical(const std::shared_ptr<physics::PhysicalObject>& po) override;

  virtual std::shared_ptr<physics::PhysicalSphere> getPhysicalSphere() const;

  virtual void setPhysicalSphere(const std::shared_ptr<physics::PhysicalSphere>& po);

  virtual std::list<std::shared_ptr<NeuriteElement>> getNeuriteList() const;

  virtual bool isANeuriteElement() const override;

  virtual bool isASomaElement() const override;

 private:
  SomaElement(const SomaElement&) = delete;
  SomaElement& operator=(const SomaElement&) = delete;

  std::shared_ptr<physics::PhysicalSphere> physical_;

  void init();
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_SOMA_ELEMENT_H_
