#ifndef PHYSICS_PHYSICAL_BOND_H_
#define PHYSICS_PHYSICAL_BOND_H_

#include <array>
#include <memory>

#include "sim_state_serializable.h"

namespace cx3d {
namespace physics {

class PhysicalObject;
class PhysicalCylinder;

/**
 * This class represents an elastic bond between two physical objects.
 * It can be used (1) to represent a cell adhesion mechanism - zip/anchor- and
 * in this case is permanent, or (2) to force two cylinders that crossed
 * each other to come back on the right side, and in this case it vanishes
 * when the right conformation is re-established.
 *
 * It works as a spring, with
 * a resting length and a spring constant, used to compute a force along the vector
 * joining the two ends, depending on the actual length. (Note that it is considered as a
 * real unique spring and not a linear spring constant as in PhysicalCylinder)
 */
class PhysicalBond : public SimStateSerializable, public std::enable_shared_from_this<PhysicalBond> {
 public:
  static std::shared_ptr<PhysicalBond> create();

  static std::shared_ptr<PhysicalBond> create(PhysicalObject* a, PhysicalObject* b);

  static std::shared_ptr<PhysicalBond> create(PhysicalObject* a,
                                              const std::array<double, 2>& position_on_a,
                                              PhysicalObject* b,
                                              const std::array<double, 2>& position_on_b, double resting_length,
                                              double spring_constant);

  PhysicalBond();

  virtual ~PhysicalBond();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual PhysicalObject* getFirstPhysicalObject();

  virtual PhysicalObject* getSecondPhysicalObject();

  virtual void setFirstPhysicalObject(PhysicalObject* a);

  virtual void setSecondPhysicalObject(PhysicalObject* b);

  /** If false, the first PhysicalObject doesn't feel the influence of this PhysicalBond.*/
  virtual bool hasEffectOnA();

  /** If false, the first PhysicalObject doesn't feel the influence of this PhysicalBond.*/
  virtual void setHasEffectOnA(bool has_effect_on_a);

  /** If false, the second PhysicalObject doesn't feel the influence of this PhysicalBond.*/
  virtual bool hasEffectOnB();

  /** If false, the second PhysicalObject doesn't feel the influence of this PhysicalBond.*/
  virtual void setHasEffectOnB(bool has_effect_on_b);

  /** If true, allows the physical bond to "slide" from b to b's mother or daughter left,
   * if b is a chain of PhysicalCylinders. It can be seen as the migration of a along b.*/
  virtual bool isSlidingAllowed();
  /**
   * If true, allows the physical bond to "slide" from b to b's mother or daughter left,
   * if b is a chain of PhysicalCylinders. It can be seen as the migration of a along b.
   * @param slidingAllowed
   */
  virtual void setSlidingAllowed(bool sliding_allowed);

  virtual void exchangePhysicalObject(PhysicalObject* oldPo, PhysicalObject* newPo);

  virtual void vanish();

  virtual PhysicalObject* getOppositePhysicalObject(PhysicalObject* po);

  virtual void setPositionOnObjectInLocalCoord(PhysicalObject* po,
                                               const std::array<double, 2>& positionInLocalCoordinates);

  virtual std::array<double, 2> getPositionOnObjectInLocalCoord(PhysicalObject* po);

  /**
   * Returns the force that this PhysicalBond is applying to a PhsicalObject.
   * The function also returns the proportion of the mass that is applied to the
   * proximal end (mother's point mass) in case of PhysicalCylinder.
   * (For PhysicalSpheres, the value p is meaningless).
   *
   * @param po the PhysicalObject to which the force is being applied.
   * @return [Fx,Fy,Fz,p] an array with the 3 force components and the proportion
   * applied to the proximal end - in case of a PhysicalCylinder.
   */
  std::array<double, 4> getForceOn(PhysicalObject* po);

  /**
   * Gets the location in absolute cartesian coord of the first insertion point (on a).
   * (Only for graphical display).Raises a NullPointerException if a == null.
   * @return x,y,z coord of the insertion point of one end
   */
  virtual std::array<double, 3> getFirstEndLocation();

  /**
   * Gets the location in absolute cartesian coord of the first insertion point (on a).
   * (Only for graphical display). Raises a NullPointerException if b == null.
   * @return x,y,z coord of the insertion point of one end
   */
  virtual std::array<double, 3> getSecondEndLocation();

  /**
   * @return the restingLength
   */
  virtual double getRestingLength();

  /**
   * @param restingLength the restingLength to set
   */
  virtual void setRestingLength(double resting_length);

  /**
   * @return the springConstant
   */
  virtual double getSpringConstant();

  /**
   * @param springConstant the springConstant to set
   */
  virtual void setSpringConstant(double spring_constant);

  /**
   * @return the maxTension
   */
  virtual double getMaxTension();

  /**
   * @param maxTension the maxTension to set
   */
  virtual void setMaxTension(double max_tension);

  /**
   * @return the dumpingConstant
   */
  virtual double getDumpingConstant();

  /**
   * @param dumpingConstant the dumpingConstant to set
   */
  virtual void setDumpingConstant(double dumping_constant);

  /**
   * Returns a String representation of this PhysicalNodeMovementListener
   */
  virtual std::string toString() const;

  virtual bool equalTo(const std::shared_ptr<PhysicalBond>& other) const;

 private:
  PhysicalObject* a_ = nullptr;
  PhysicalObject* b_ = nullptr;
  ;
  std::array<double, 2> origin_on_a_;
  std::array<double, 2> origin_on_b_;
  double resting_length_ = 0;
  double spring_constant_ = 10;
  double max_tension_ = 50;
  double dumping_constant_ = 0;

  double past_length_ = 0;

  /** If true, allows the physical bond to "slide" if b is a chain of PhysicalCylinders
   * It can be seen as the migration of a along b. */
  bool sliding_allowed_ = false;
  /** If false, there is no force transmitted on the first PhysicalObject (a).*/
  bool has_effect_on_a_ = true;
  /** If false, there is no force transmitted on the second PhysicalObject (b).*/
  bool has_effect_on_b_ = true;

  void dolocking(PhysicalObject* a, PhysicalObject* b);

  // necessary because initialization code uses this->shared_from_this() - cannot be in constructor
  void init(PhysicalObject* a, PhysicalObject* b);

  void init(PhysicalObject* a, const std::array<double, 2>& position_on_a, PhysicalObject* b,
            const std::array<double, 2>& position_on_b, double resting_length,
            double spring_constant);
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_BOND_H_
