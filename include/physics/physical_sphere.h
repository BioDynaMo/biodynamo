#ifndef PHYSICS_PHYSICAL_SPHERE_H_
#define PHYSICS_PHYSICAL_SPHERE_H_

#include <list>
#include <array>
#include <memory>
#include <unordered_map>

//#include "physics/physical_object.h"
#include "physics/physical_cylinder.h"

namespace cx3d {

namespace local_biology {
class SomaElement;
class CellElement;
}  // namespace local_biology

namespace physics {

using local_biology::SomaElement;

/**
 * The embodiment of SomaElement. Contains
 *
 * The spherical coordinates (r, phi, theta) are defined as:
 * r >= 0 is the distance from the origin to a given point P.
 * 0 <= phi <= pi is the angle between the positive z-axis and the line formed between the origin and P.
 * 0 <= theta < 2pi is the angle between the positive x-axis and the line from the origin
 * to the P projected onto the xy-plane.
 */
class PhysicalSphere : public PhysicalObject {
 public:
  using UPtr = std::unique_ptr<PhysicalSphere>;

  PhysicalSphere();  //todo should be protected

  virtual ~PhysicalSphere() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual double getInterObjectForceCoefficient() const override;

  virtual void setInterObjectForceCoefficient(double inter_object_force_coefficient) override;

  virtual double getRotationalInertia() const;

  virtual void setRotationalInertia(double rotational_inertia);

  /** returns true because this object is a PhysicalSphere */
  virtual bool isAPhysicalSphere() const override;

  virtual void movePointMass(double speed, const std::array<double, 3>& direction) override;

  virtual std::array<double, 3> originOf(PhysicalObject* daughter) override;

  // *************************************************************************************
  //   BIOLOGY (growth, division, new neurite... )
  // *************************************************************************************

  /**
   * @return the somaElement
   */
  virtual SomaElement* getSomaElement() const;

  /**
   * @param somaElement the somaElement to set
   */
  virtual void setSomaElement(SomaElement* soma_element);

  /**
   * Progressive modification of the volume. Updates the diameter, the intracellular concentration
   * @param speed cubic micron/ h
   */
  virtual void changeVolume(double speed) override;

  /**
   * Progressive modification of the diameter. Updates the volume, the intracellular concentration
   * @param speed micron/ h
   */
  virtual void changeDiameter(double speed) override;

  /**
   * Extension of a PhysicalCylinder as a daughter of this PhysicalSphere. The position on the sphere where
   * the cylinder will be attached is specified in spherical coordinates with respect to the
   * cx3d.cells Axis with two angles. The cylinder that is produced is specified by the object (usually
   * a SomaElement) that calls this method. Indeed, unlike PhysicalCylinder.insertProximalCylinder() for instance,
   * this method is called for biological reasons, and not for discretization purposes.
   *
   * @param cyl the PhysicalCylinder instance (or a class derived from it) that will be extended.
   * @param phi the angle from the zAxis
   * @param theta the angle from the xAxis around the zAxis
   */
  virtual PhysicalCylinder::UPtr addNewPhysicalCylinder(double new_length, double phi, double theta);

  /**
   * Division of the sphere into two spheres. The one in which the method is called becomes
   * one the 1st daughter sphere (it keeps its Soma); a new PhysicalSphere is instantiated
   * and becomes the 2nd daughter (and the Soma given as argument is attributed to it
   * as CellElement). One can specify the relative size of the daughters (2nd/1st).
   * In asymmetrical division the cx3d.cells that divides stays the progenitor, so the ratio is
   * smaller than 1.
   * @param somaElement the PhysicalSphere for daughter 2
   * @param vr ratio of the two volumes (vr = v2/v1)
   * @param phi the angle from the zAxis (for the division axis)
   * @param theta the angle from the xAxis around the zAxis (for the division axis)
   * @return the other daughter (new sphere)
   */
  virtual UPtr divide(double vr, double phi, double theta);

  // *************************************************************************************
  //   PHYSICS
  // *************************************************************************************

  /**
   * Tells if a sphere is in the detection range of an other sphere.
   */
  virtual bool isInContactWithSphere(PhysicalSphere* s) override;

  virtual bool isInContactWithCylinder(PhysicalCylinder* c) override;

  virtual std::array<double, 4> getForceOn(PhysicalCylinder* c) override;

  virtual std::array<double, 3> getForceOn(PhysicalSphere* s) override;

  virtual void runPhysics() override;

  virtual std::array<double, 3> getAxis() const override;

  /**
   * @return the daughters
   */
  virtual std::list<PhysicalCylinder*> getDaughters() const;

  virtual void runIntracellularDiffusion() override;

  // *************************************************************************************
  //   Coordinates transform
  // *************************************************************************************

  /*
   * 3 systems of coordinates :
   *
   * Global :   cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0) and (0,0,1)
   *        with origin at (0,0,0).
   * Local :    defined by orthogonal axis xAxis, yAxis and zAxis,
   *        with origin at center of the sphere (point mass location)
   * Polar :    spherical coordinates [r,phi,theta] with
   *        r >= 0 is the distance between the origin (point mass) O and a given point P,
   *        0 <= phi <= pi is the angle between the positive z-axis and the line OP.
   *        0 <= theta < 2pi is the angle between the positive x-axis and the projection of OP onto the xy-plane
   *
   *
   *  Note: The methods below transform POSITIONS and not DIRECTIONS !!!
   *
   * G -> L
   * L -> G
   *
   * L -> P
   * P -> L
   *
   * G -> P = G -> L, then L -> P
   * P -> P = P -> L, then L -> G
   */

  // G -> L
  /**
   * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
   * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
   * @param positionInGlobalCoord
   * @return
   */
  virtual std::array<double, 3> transformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const override;

  // L -> G
  /**
   * Returns the position in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
   * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
   * @param positionInLocalCoord
   * @return
   */
  virtual std::array<double, 3> transformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const override;

  // L -> P
  /**
   * Returns the position in spherical coordinates (r,phi,theta)
   * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
   * @param positionInLocalCoord
   * @return
   */
  virtual std::array<double, 3> transformCoordinatesLocalToPolar(const std::array<double, 3>& position) const;

  // P -> L
  /**
   * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
   * of a point expressed in spherical coordinates (r,phi,theta).
   * @param positionInLocalCoord
   * @return
   */
  virtual std::array<double, 3> transformCoordinatesPolarToLocal(const std::array<double, 3>& position) const;

  virtual std::array<double, 3> transformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const override;

  virtual std::array<double, 3> transformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const override;

  virtual std::array<double, 3> getUnitNormalVector(const std::array<double, 3>& position) const override;

  virtual CellElement* getCellElement() const override;

  virtual bool isRelative(PhysicalObject* po) const override;

  virtual double getLength() const override;

//fixme swig forces following methods to be public -> change to  protected:
  /**
   * A PhysicalSphere has no mother that could call, so this method is not used.
   */
  virtual std::array<double, 3> forceTransmittedFromDaugtherToMother(PhysicalObject* mother)
      override;

  virtual void removeDaugther(PhysicalObject* daughter) override;

  virtual void updateRelative(PhysicalObject* old_relative, PhysicalObject* new_relative) override;

  virtual void updateDependentPhysicalVariables() override;

  /**
   * Updates the concentration of substances, based on the volume of the object.
   * Is usually called after change of the volume (and therefore we don't modify it here)
   */
  virtual void updateIntracellularConcentrations() override;

  /**
   * Recompute volume after diameter has changed.
   */
  virtual void updateVolume() override;

  virtual void updateDiameter() override;

  virtual std::string toString() const override;

 private:
  PhysicalSphere(const PhysicalSphere& other) = delete;
  PhysicalSphere& operator=(const PhysicalSphere& other) = delete;

  /** Local biology object associated with this PhysicalSphere.*/
  SomaElement* soma_element_ = nullptr;

  /* The PhysicalCylinders attached to this sphere*/
  std::list<PhysicalCylinder*> daughters_;
  /* Position in local coordinates (PhysicalObject*'s xAxis,yAxis,zAxis) of
   * the attachment point of my daughters.*/
  std::unordered_map<PhysicalCylinder*, std::array<double, 3>, PhysicalCylinderHash,
      PhysicalCylinderEqual> daughters_coord_;

  /* Plays the same role than mass and adherence, for rotation around center of mass. */
  double rotational_inertia_ = 0.5;  // 5.0  //todo hardcoded parameters - better in config file

  double inter_object_force_coefficient_ = 0.15;  //todo hardcoded parameters - better in config file

  /* Force applied by the biology. Is taken into account during runPhysics(), and the set to 0.*/
  std::array<double, 3> tractor_force_ = std::array<double, 3> { 0, 0, 0 };

  /* Move the SpatialOrganizationNode in the center of this PhysicalSphere. If it is
   * only a small distance off (half of the radius), there is no movement.
   */
  void updateSpatialOrganizationNodePosition();

  void scheduleMeAndAllMyFriends();
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_SPHERE_H_
