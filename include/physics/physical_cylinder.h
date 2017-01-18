#ifndef PHYSICS_PHYSICAL_CYLINDER_H_
#define PHYSICS_PHYSICAL_CYLINDER_H_

#include <array>
#include <memory>
#include <exception>

#include "physics/physical_object.h"

namespace bdm {

namespace local_biology {
class CellElement;
class NeuriteElement;
}  // namespace local_biology

namespace physics {

using local_biology::CellElement;
using local_biology::NeuriteElement;

/**
 * A cylinder can be seen as a normal cylinder, with two end points and a diameter. It is oriented;
 * the two points are called proximal and distal. The PhysicalCylinder is be part of a tree-like
 * structure with (one and only) one Physical object at its proximal point and (up to) two physical Objects at
 * its distal end. If there is only one daughter,
 * it is the left one. If <code>daughterLeft == null</code>, there is no distal cylinder (this
 * is a terminal cylinder). The presence of a <code>daugtherRight</code> means that this branch has a bifurcation
 * at its distal end.
 * <p>
 * All the mass of this cylinder is concentrated at the distal point. Only the distal end is moved
 * by a PhysicalCylinder. All the forces in a cylinder that are applied to the proximal node (belonging to the
 * mother PhysicalNode) are transmitted to the mother element
 */
class PhysicalCylinder : public PhysicalObject {
 public:
  using UPtr = std::unique_ptr<PhysicalCylinder>;

  PhysicalCylinder();

  virtual ~PhysicalCylinder();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  std::string toString() const override;

  /** Returns a <code>PhysicalCylinder</code> with all fields similar than in this
   * <code>PhysicalCylinder</code>. Note that the relatives in the tree structure, the
   * tension, the volume, the
   * <code>CellElement</code>, as well as<code>Excrescences</code> and the
   * <code>IntracellularSubstances</code> are not copied. */
  UPtr getCopy() const;

  // *************************************************************************************
  // *      METHODS FOR NEURON TREE STRUCTURE                                            *
  // *************************************************************************************

  /**
   * Returns true if the <code>PhysicalObject</code> given as argument is a mother, daughter
   * or sister branch.*/
  bool isRelative(PhysicalObject* po) const override;

  /**
   * Returns the location in absolute coordinates of where the <code>PhysicalObject</code>
   * given as argument is attached on this where the <code>PhysicalCylinder</code>
   * If the argument is one of our daughter <code>PhysicalCylinder</code>, the point mass location
   * is returned. Otherwise, the return is <code>null</code>.
   *
   * @param daughterWhoAsks the PhysicalObject requesting it's origin.
   *
   */
  std::array<double, 3> originOf(PhysicalObject* daughter) override;

  void removeDaughter(PhysicalObject* daughter) override;

  void updateRelative(PhysicalObject* old_relative, PhysicalObject* new_relative) override;

  /**
   * returns the total force that this <code>PhysicalCylinder</code> exerts on it's mother.
   * It is the sum of the spring force an the part of the inter-object force computed earlier in
   * <code>runPhysics()</code>.
   */
  std::array<double, 3> forceTransmittedFromDaugtherToMother(PhysicalObject* mother) override;

  // *************************************************************************************
  //   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
  // *************************************************************************************

  /**
   * Checks if this <code>PhysicalCylinder</code> is either too long (and in this case it will insert
   * another <code>PhysicalCylinder</code>), or too short (and in this second case fuse it with the
   * proximal element or even delete it).
   * */
  bool runDiscretization();

  // *************************************************************************************
  //   ELONGATION, RETRACTION, BRANCHING
  // *************************************************************************************

  /** Method used for active extension of a terminal branch, representing the steering of a
   * growth cone. The movement should always be forward, otherwise no movement is performed.
   *
   * @param speed of the growth rate (microns/hours).
   * @direction the 3D direction of movement.
   */
  void extendCylinder(double speed, const std::array<double, 3>& direction);

  /** Method used for active extension of a terminal branch, representing the steering of a
   * growth cone. There is no check for real extension (unlike in extendCylinder() ).
   *
   * @param speed of the growth rate (microns/hours).
   * @direction the 3D direction of movement.
   */
  void movePointMass(double speed, const std::array<double, 3>& direction) override;

  /**
   * Branch retraction by moving the distal end (i.e. the massLocation) toward the
   * proximal end (the mother), maintaining the same tension in the PhysicalCylinder. The method
   * shortens the actual and the resting length so that the result is a shorter
   * cylinder with the same tension.
   * - If this PhysicalCylinder is longer than the required shortening, it simply retracts.
   * - If it is shorter and its mother has no other daughter, it merges with it's mother and
   * the method is recursively called (this time the cylinder length is bigger because we have
   * a new PhysicalCylinder that resulted from the fusion of two).
   * - If it is shorter and either the previous PhysicalCylinder has another daughter or the
   * mother is not a PhysicalCylinder, it disappears.
   *
   * @param speed of the retraction (microns/hours).
   * @return false if the neurite doesn't exist anymore (complete retraction)
   */
  bool retractCylinder(double speed);

  /**
   * Bifurcation of the growth cone creating : adds the 2 <code>PhysicalCylinder</code> that become
   * daughter left and daughter right
   * @param length the length of the new branches
   * @param direction_1 of the first branch (if
   * @param newBranchL
   * @param newBranchR
   */

  std::array<UPtr, 2> bifurcateCylinder(double length, const std::array<double, 3>& direction_1,
                                        const std::array<double, 3>& direction_2);

  /**
   * Makes a side branching by adding a second daughter to a non terminal <code>PhysicalCylinder</code>.
   * The new <code>PhysicalCylinder</code> is perpendicular to the mother branch.
   * @param direction the direction of the new neuriteLement (But will be automatically corrected if
   * not al least 45 degrees from the cylinder's axis).
   * @return the newly added <code>NeuriteSegment</code>
   */
  UPtr branchCylinder(double length, const std::array<double, 3>& direction);

  void setRestingLengthForDesiredTension(double tension);

  /**
   * Progressive modification of the volume. Updates the diameter, the intracellular concentration
   * @param speed cubic micron/ h
   */
  void changeVolume(double speed) override;

  /**
   * Progressive modification of the diameter. Updates the volume, the intracellular concentration
   * @param speed micron/ h
   */
  void changeDiameter(double speed) override;

  // *************************************************************************************
  //   Physics
  // *************************************************************************************

  void runPhysics() override;

  std::array<double, 3> getForceOn(PhysicalSphere* s) override;

  std::array<double, 4> getForceOn(PhysicalCylinder* c) override;

  bool isInContactWithSphere(PhysicalSphere* s) override;

  bool isInContactWithCylinder(PhysicalCylinder* c) override;

  /** Returns the point on this cylinder's spring axis that is the closest to the point p.*/
  std::array<double, 3> closestPointTo(const std::array<double, 3>& p);

  void runIntracellularDiffusion() override;

  std::array<double, 3> getUnitNormalVector(const std::array<double, 3>& position) const override;

  /**
   * Defines the three orthonormal local axis so that a cylindrical coordinate system
   * can be used. The xAxis is aligned with the springAxis. The two other are in the
   * plane perpendicular to springAxis. This method to update the axis was suggested by
   * Matt Coock. - Although not perfectly exact, it is accurate enough for us to use.
   *
   */
  void updateLocalCoordinateAxis();

  /** Recomputes diameter after volume has changed.*/
  void updateDiameter() override;

  /** Recomputes volume, after diameter has been change. And makes a call for
   * recomputing then concentration of IntracellularSubstances.*/
  void updateVolume() override;

  // *************************************************************************************
  //   Coordinates transform
  // *************************************************************************************

  /**
   * 3 systems of coordinates :
   *
   * Global :   cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0) and (0,0,1)
   *        with origin at (0,0,0).
   * Local :    defined by orthogonal axis xAxis (=vect proximal to distal end), yAxis and zAxis,
   *        with origin at proximal end
   * Polar :    cylindrical coordinates [h,theta,r] with
   *        h = first local coord (along xAxis),
   *        theta = angle from yAxis,
   *        r euclidian distance from xAxis;
   *        with origin at proximal end
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

  /**
   * G -> L
   * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
   * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
   * @param positionInGlobalCoord
   * @return
   */
  std::array<double, 3> transformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const override;

  /**
   * L -> G
   * Returns the position in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
   * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
   * @param positionInLocalCoord
   * @return
   */
  std::array<double, 3> transformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const override;

  /**
   *  L -> P
   * Returns the position in cylindrical coordinates (h,theta,r)
   * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
   * @param positionInLocalCoord
   * @return
   */
  std::array<double, 3> transformCoordinatesLocalToPolar(const std::array<double, 3>& position) const;
  /**
   * P -> L
   * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
   * of a point expressed in cylindrical coordinates (h,theta,r).
   * @param positionInLocalCoord
   * @return
   */
  std::array<double, 3> transformCoordinatesPolarToLocal(const std::array<double, 3>& position) const;

  /** P -> G :    P -> L, then L -> G */
  std::array<double, 3> transformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const override;

  /** G -> L :    G -> L, then L -> P */
  std::array<double, 3> transformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const override;

  // *************************************************************************************
  //   GETTERS & SETTERS
  // *************************************************************************************

  /** Well, there is no field cellElement. We return neuriteElement.*/
  CellElement* getCellElement() const override;

  /**
   * @return the neuriteElement
   */
  NeuriteElement* getNeuriteElement() const;

  /**
   * @param neuriteElement the neuriteElement to set
   */
  void setNeuriteElement(NeuriteElement* neurite);

  /**
   * @return the daughterLeft
   */
  PhysicalCylinder* getDaughterLeft() const;

  /**
   * @return the daughterRight
   */
  PhysicalCylinder* getDaughterRight() const;

  /**
   * @return the mother
   */
  PhysicalObject* getMother() const;

  /**
   * @param mother the mother to set
   */
  void setMother(PhysicalObject* mother);

  /**
   * @param daughterLeft the daughterLeft to set
   */
  void setDaughterLeft(PhysicalCylinder* daughter_left);

  /**
   * @param daughterRight the daughterRight to set
   */
  void setDaughterRight(PhysicalCylinder* daughter_right);

  /**
   * @param branchOrder the branchOrder to set
   */
  void setBranchOrder(int branch_order);

  /**
   * @return the branchOrder
   */
  int getBranchOrder() const;

  double getActualLength() const;

  /**
   * Should not be used, since the actual length depends on the geometry.
   * @param actualLength
   */
  void setActualLength(double actual_length);

  double getRestingLength() const;

  void setRestingLength(double resting_length);

  std::array<double, 3> getSpringAxis() const;

  void setSpringAxis(const std::array<double, 3>& axis);

  double getSpringConstant() const;

  void setSpringConstant(double spring_constant);

  double getTension() const;

  void setTension(double tension);

  /**
   * NOT A "REAL" GETTER
   * Gets a vector of length 1, with the same direction as the SpringAxis.
   * @return a normalized springAxis
   */
  std::array<double, 3> getUnitaryAxisDirectionVector() const;

  /**
   * Should return yes if the PhysicalCylinder is considered a terminal branch.
   * @return is it a terminal branch
   */
  bool isTerminal() const;

  /**
   * Returns true if a bifurcation is physicaly possible. That is if the PhysicalCylinder
   * has no daughter and the actual length is bigger than the minimum required.
   * @return
   */
  bool bifurcationPermitted() const;

  /**
   * Returns true if a side branch is physically possible. That is if this is not a terminal
   * branch and if there is not already a second daughter.
   * @return
   */
  bool branchPermitted() const;

  /**
   * retuns the position of the proximal end, ie the massLocation minus the spring axis.
   * Is mainly used for paint
   * @return
   */
  std::array<double, 3> proximalEnd() const;

  /**
   * retuns the position of the distal end, ie the massLocation coordinates (but not the
   * actual massLocation array).
   * Is mainly used for paint
   * @return
   */
  std::array<double, 3> distalEnd() const;

  /**
   * Returns the total (actual) length of all the cylinders (including the one in which this method is
   * called) before the previous branching point. Used to decide if long enough to bifurcate or branch,
   * independently of the discretization.
   * @return
   */
  double lengthToProximalBranchingPoint() const;

  /** returns true because this object is a PhysicalCylinder */
  bool isAPhysicalCylinder() const override;

  double getLength() const override;

  double getInterObjectForceCoefficient() const override;

  void setInterObjectForceCoefficient(double coefficient) override;

  std::array<double, 3> getAxis() const override;

  /**
   * Updates the spring axis, the actual length, the tension and the volume.
   *
   * For tension, T = k*(aL-rL)/rL.  k = spring constant,
   * rL = resting length, aL = actual length. (Note the division by rL.
   * Otherwise we could have Cylinders with big aL and rL = 0).
   * <p>
   * This method also automatically calls the <code>resetComputationCenterPosition()</code>
   * method at the end.
   */
  void updateDependentPhysicalVariables() override;

  void checkpoint(std::vector<double>* data, std::vector<double>* checkpoint, size_t object_number) const {
    static int c = 8;
    if (checkpoint != nullptr) {
      (*data)[object_number * c + 0] = mass_location_[0] - (*checkpoint)[object_number * c + 0];
      (*data)[object_number * c + 1] = mass_location_[1] - (*checkpoint)[object_number * c + 1];
      (*data)[object_number * c + 2] = mass_location_[2] - (*checkpoint)[object_number * c + 2];
      (*data)[object_number * c + 3] = diameter_  - (*checkpoint)[object_number * c + 3];
      (*data)[object_number * c + 4] = actual_length_  - (*checkpoint)[object_number * c + 4];
      (*data)[object_number * c + 5] = spring_axis_[0] - (*checkpoint)[object_number * c + 5];
      (*data)[object_number * c + 6] = spring_axis_[1] - (*checkpoint)[object_number * c + 6];
      (*data)[object_number * c + 7] = spring_axis_[2] - (*checkpoint)[object_number * c + 7];

      // (*data)[object_number * c + 0] = (double) ((long long) (mass_location_[0]) ^ ( (long long) (*checkpoint)[object_number * c + 0]));
      // (*data)[object_number * c + 1] = (double) ((long long) (mass_location_[1]) ^ ( (long long) (*checkpoint)[object_number * c + 1]));
      // (*data)[object_number * c + 2] = (double) ((long long) (mass_location_[2]) ^ ( (long long) (*checkpoint)[object_number * c + 2]));
      // (*data)[object_number * c + 3] = (double) ((long long) diameter_  ^ ( (long long) (*checkpoint)[object_number * c + 3]));
      // (*data)[object_number * c + 4] = (double) ((long long) actual_length_  ^ ( (long long)  (*checkpoint)[object_number * c + 4]));
      // (*data)[object_number * c + 5] = (double) ((long long) spring_axis_[0] ^ ( (long long) (*checkpoint)[object_number * c + 5]));
      // (*data)[object_number * c + 6] = (double) ((long long) spring_axis_[1] ^ ( (long long) (*checkpoint)[object_number * c + 6]));
      // (*data)[object_number * c + 7] = (double) ((long long) spring_axis_[2] ^ ( (long long) (*checkpoint)[object_number * c + 7]));
    }
  }

 protected:
  /**
   * Updates the concentration of substances, based on the volume of the object.
   * Is usually called after change of the volume (and therefore we don't modify it here)
   */
  void updateIntracellularConcentrations() override;

  /**
   * Repositioning of the SpatialNode location (usually a Delaunay vertex) at the barycenter of the cylinder.
   * If it is already closer than a quarter of the diameter of the cylinder, it is not displaced.
   */
  void updateSpatialOrganizationNodePosition();

 private:
  PhysicalCylinder(const PhysicalCylinder& other) = delete;
  PhysicalCylinder& operator=(const PhysicalCylinder& other) = delete;

  /** Local biology object associated with this PhysicalCylinder.*/
  NeuriteElement* neurite_element_ = nullptr;

  /** Parent node in the neuron tree structure (can be PhysicalSphere or PhysicalCylinder)*/
  PhysicalObject* mother_ = nullptr;

  /** First child node in the neuron tree structure (can only be PhysicalCylinder)*/
  PhysicalCylinder* daughter_left_ = nullptr;

  /** Second child node in the neuron tree structure. (only PhysicalCylinder) */
  PhysicalCylinder* daughter_right_ = nullptr;

  /** number of branching points from here to the soma (root of the neuron tree-structure).*/
  int branch_order_ = 0;

  /** The part of the inter-object force transmitted to the mother (parent node) -- c.f. runPhysics() */
  std::array<double, 3> force_to_transmit_to_proximal_mass_ = std::array<double, 3> { 0, 0, 0 };

  /** Vector from the attachment point to the massLocation (proximal -> distal).  */
  std::array<double, 3> spring_axis_ = std::array<double, 3> { 0, 0, 0 };

  /** Real length of the PhysicalCylinder (norm of the springAxis). */
  double actual_length_ = Param::kNeuriteDefaultActualLength;

  /** Tension in the cylinder spring.*/
  double tension_ = Param::kNeuriteDefaultTension;

  /** Spring constant per distance unit (springConstant restingLength  = "real" spring constant). */
  double spring_constant_ = Param::kNeuriteDefaultSpringConstant;

  /** The length of the internal spring where tension would be zero. */
  double resting_length_ = spring_constant_ * actual_length_ / (tension_ + spring_constant_);  // T = k*(A-R)/R --> R = k*A/(T+K)

  /**
   * Divides the PhysicalCylinder into two PhysicalCylinders of equal length. The one in which the method is called becomes the distal half.
   * A new PhysicalCylinder is instantiated and becomes the proximal part. All characteristics are transmitted.
   * A new Neurite element is also instantiated, and assigned to the new proximal PhysicalCylinder
   */
  NeuriteElement* insertProximalCylinder();

  /**
   * Divides the PhysicalCylinder into two PhysicalCylinders (in fact, into two instances of the derived class).
   * The one in which the method is called becomes the distal half, and it's length is reduced.
   * A new PhysicalCylinder is instantiated and becomes the proximal part (=the mother). All characteristics are transmitted
   *
   * @param distalPortion the fraction of the total old length devoted to the distal half (should be between 0 and 1).
   */
  NeuriteElement* insertProximalCylinder(double distal_portion);

  /**
   * Merges two Cylinders together. The one in which the method is called phagocytes it's mother.
   * The CellElement of the PhysicalCylinder that is removed is also removed: it's removeYourself() method is called.
   */
  void removeProximalCylinder();

  /**
   * Sets the scheduling flag onTheSchedulerListForPhysicalObjects to true
   * for me and for all my neighbors, relative, things I share a physicalBond with
   */
  void scheduleMeAndAllMyFriends();

  UPtr extendSideCylinder(double length, const std::array<double, 3>& direction);
};

struct PhysicalCylinderHash {
  std::size_t operator()(PhysicalCylinder* element) const {
    return reinterpret_cast<std::size_t>(element);
  }
};

struct PhysicalCylinderEqual {
  bool operator()(PhysicalCylinder* lhs, PhysicalCylinder* rhs) const {
    return lhs == rhs;
  }
};

}  //namespace physics
}  //namespace bdm

#endif  // PHYSICS_PHYSICAL_CYLINDER_H_
