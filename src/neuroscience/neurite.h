#ifndef NEUROSCIENCE_NEURITE_H_
#define NEUROSCIENCE_NEURITE_H_

#include "param.h"
#include "simulation_object.h"
#include "simulation_object_util.h"

namespace bdm {

// TODO
/// The mother of a neurite can either be a neuron or a neurite.
/// Therefore, this class acts as an intermediate layer that forwards function
/// calls to the correct object.
/// @tparam TNeuron type of neuron
/// @tparam TNeurite type of neurite. Needs to be a template parameter since
///         Neurite is not defined at this point
// template<typename TNeuron, typename TNeurite>
// class NeuriteMother {
// public:
//   template <typename T>
//   typename std::enable_if<is_same<T, Neuron>::value>::type
//   Set(T&& so) {
//     neuron_ptr_ = so;
//   }
//
//   template <typename T>
//   typename std::enable_if<is_same<T, Neurite>::value>::type
//   Set(T&& so) {
//     neurite_ptr_ = so;
//   }
//
//   const std::array<double, 3>& GetPosition() const {
//     if (!neurite_ptr_.Get().IsNullPtr()) {
//       return neurite_ptr_.Get().GetPosition();
//     }
//     return neuron_ptr_.Get().GetPosition();
//   }
//
//   bool IsNeuron() const { return !neuron_ptr_.IsNullPtr(); }
//   bool IsNeurite() const { return !neurite_ptr_.IsNullPtr(); }
//
//   auto GetDaughterLeft() -> decltype(neurite_ptr_.Get().GetDaughterLeft()) const {
//     assert(IsNeurite());
//     return neurite_ptr_.Get().GetDaughterLeft();
//   }
//
//   template <typename T>
//   void RemoveDaughter(T* mother) {
//     assert(IsNeuron());
//     // FIXME
//     // neuron_ptr_.Get().RemoveDaughter();
//   }
//
// private:
//   SoPointer<TNeuron, > neuron_ptr_;
//   SoPointer<TNeurite> neurite_ptr_;
// };

/// Class defining the biological properties of a neurite segment, if it contains
/// a <code>LocalBiologyModule</code>. This class is associated with a <code>PhysicalCylinder</code>.
///
///
/// A cylinder can be seen as a normal cylinder, with two end points and a diameter. It is oriented;
/// the two points are called proximal and distal. The PhysicalCylinder is be part of a tree-like
/// structure with (one and only) one Physical object at its proximal point and (up to) two physical Objects at
/// its distal end. If there is only one daughter,
/// it is the left one. If <code>daughterLeft == null</code>, there is no distal cylinder (this
/// is a terminal cylinder). The presence of a <code>daugtherRight</code> means that this branch has a bifurcation
/// at its distal end.
/// <p>
/// All the mass of this cylinder is concentrated at the distal point. Only the distal end is moved
/// by a PhysicalCylinder. All the forces in a cylinder that are applied to the proximal node (belonging to the
/// mother PhysicalNode) are transmitted to the mother element
///
BDM_SIM_OBJECT(Neurite, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(NeuriteExt, 1, daughter_left_, daughter_right_, spring_axis_, actual_length_, tension_, spring_constant_, resting_length_);
 public:
   NeuriteExt() {}

  /// Retracts the Cylinder associated with this NeuriteElement, if it is a terminal one.
  /// \param speed the retraction speed in microns / h
  // void RetractTerminalEnd(double speed);

 //  /// Moves the point mass of the Cylinder associated with this NeuriteElement, if it is a terminal one.
 //  ///  BUT : if "direction" points in an opposite direction than the cylinder axis, i.e.
 //  ///  if the dot product is negative, there is no movement (only elongation is possible).
 //  /// \param speed
 //  /// \param direction
 //  ///
 //  void elongateTerminalEnd(double speed, const std::array<double, 3>& direction);
 //
 //  ///
 //  /// Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
 //  /// \param newBranchDiameter
 //  /// \param growthDirection (But will be automatically corrected if not at least 45 degrees from the cylinder's axis).
 //  /// \return
 //  ///
 //  NeuriteElement* branch(double newBranchDiameter, const std::array<double, 3>& direction);
 //
 //  ///
 //  /// Makes a side branch, i.e. splits this cylinder into two and puts a daughteRight at the proximal half.
 //  /// \param growthDirection (But will be automatically corrected if not at least 45 degrees from the cylinder's axis).
 //  /// \return
 //  ///
 //  NeuriteElement* branch(const std::array<double, 3>& direction);
 //
 //  ///
 //  /// Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
 //  /// \param diameter of the side branch
 //  /// \return
 //  ///
 //  NeuriteElement* branch(double diameter);
 //
 //  ///
 //  /// Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
 //  /// \return
 //  ///
 //  NeuriteElement* branch();
 //
 //  ///
 //   Returns <code>true</code> if it is a terminal cylinder with length of at least 1micron.
 //  /// \return
 //  ///
 //  bool bifurcationPermitted() const;
 //
 //  ///
 //  /// Bifurcation of a growth come (only works for terminal segments).
 //  /// Note : angles are corrected if they are pointing backward.
 //  /// \param diameter_1  of new daughterLeft
 //  /// \param diameter_2 of new daughterRight
 //  /// \param direction_1
 //  /// \param direction_2
 //  /// \return
 //  ///
 //  std::array<NeuriteElement*, 2> bifurcate(double diameter_1, double diameter_2,
 //                                           const std::array<double, 3>& direction_1,
 //                                           const std::array<double, 3>& direction_2);
 //
 //  /// Bifurcation of a growth come (only works for terminal segments).
 //  /// Note : angles are corrected if they are pointing backward.
 //  /// \param length of new branches
 //  /// \param diameter_1  of new daughterLeft
 //  /// \param diameter_2 of new daughterRight
 //  /// \param direction_1
 //  /// \param direction_2
 //  /// \return
 //  std::array<NeuriteElement*, 2> bifurcate(double length, double diameter_1, double diameter_2,
 //                                           const std::array<double, 3>& direction_1,
 //                                           const std::array<double, 3>& direction_2);
 //
 //  std::array<NeuriteElement*, 2> bifurcate(const std::array<double, 3>& direction_1,
 //                                           const std::array<double, 3>& direction_2);
 //
 //  std::array<NeuriteElement*, 2> bifurcate();
 //
 //  bool isAxon() const;
 //
 //    void setAxon(bool is_axon);
 //
 //    ///
 //  /// \return the (first) distal <code>NeuriteElement</code>, if it exists,
 //  /// i.e. if this is not the terminal segment (otherwise returns <code>null</code>).
 //  ///
 //  NeuriteElement* getDaughterLeft() const;
 //
 //  ///
 //  /// \return the second distal <code>NeuriteElement</code>, if it exists
 //  /// i.e. if there is a branching point just after this element (otherwise returns <code>null</code>).
 //  ///
 //  NeuriteElement* getDaughterRight() const;
 //
 //  // TODO remove
 //  // *************************************************************************************
 //  /////     from PhysicalCylinder                            *
 //  // *************************************************************************************
 //
 //  // *************************************************************************************
 //  /////      METHODS FOR NEURON TREE STRUCTURE                                            *
 //  // *************************************************************************************
 //
 //  ///
 //  /// Returns true if the <code>PhysicalObject</code> given as argument is a mother, daughter
 //  /// or sister branch.*/
 //  bool isRelative(PhysicalObject* po) const override;
 //
 //  ///
 //  /// Returns the location in absolute coordinates of where the <code>PhysicalObject</code>
 //  /// given as argument is attached on this where the <code>PhysicalCylinder</code>
 //  /// If the argument is one of our daughter <code>PhysicalCylinder</code>, the point mass location
 //  /// is returned. Otherwise, the return is <code>null</code>.
 //  ///
 //  /// \param daughterWhoAsks the PhysicalObject requesting it's origin.
 //  ///
 //  ///
 //  std::array<double, 3> originOf(PhysicalObject* daughter) override;
 //
 //  void removeDaughter(PhysicalObject* daughter) override;
 //
 //  void updateRelative(PhysicalObject* old_relative, PhysicalObject* new_relative) override;
 //
 //  ///
 //  /// returns the total force that this <code>PhysicalCylinder</code> exerts on it's mother.
 //  /// It is the sum of the spring force an the part of the inter-object force computed earlier in
 //  /// <code>runPhysics()</code>.
 //  ///
 //  std::array<double, 3> forceTransmittedFromDaugtherToMother(PhysicalObject* mother) override;
 //
 //  // *************************************************************************************
 //  //   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
 //  // *************************************************************************************
 //
 //  ///
 //  /// Checks if this <code>PhysicalCylinder</code> is either too long (and in this case it will insert
 //  /// another <code>PhysicalCylinder</code>), or too short (and in this second case fuse it with the
 //  /// proximal element or even delete it).
 //  bool runDiscretization();
 //
 //  // *************************************************************************************
 //  //   ELONGATION, RETRACTION, BRANCHING
 //  // *************************************************************************************
 //
 //  /// Method used for active extension of a terminal branch, representing the steering of a
 //  /// growth cone. The movement should always be forward, otherwise no movement is performed.
 //  ///
 //  /// \param speed of the growth rate (microns/hours).
 //  /// @direction the 3D direction of movement.
 //  ///
 //  void extendCylinder(double speed, const std::array<double, 3>& direction);
 //
 //  /// Method used for active extension of a terminal branch, representing the steering of a
 //  /// growth cone. There is no check for real extension (unlike in extendCylinder() ).
 //  ///
 //  /// \param speed of the growth rate (microns/hours).
 //  /// @direction the 3D direction of movement.
 //  ///
 //  void movePointMass(double speed, const std::array<double, 3>& direction) override;
 //
 //  ///
 //  /// Branch retraction by moving the distal end (i.e. the massLocation) toward the
 //  /// proximal end (the mother), maintaining the same tension in the PhysicalCylinder. The method
 //  /// shortens the actual and the resting length so that the result is a shorter
 //  /// cylinder with the same tension.
 //  /// - If this PhysicalCylinder is longer than the required shortening, it simply retracts.
 //  /// - If it is shorter and its mother has no other daughter, it merges with it's mother and
 //  /// the method is recursively called (this time the cylinder length is bigger because we have
 //  /// a new PhysicalCylinder that resulted from the fusion of two).
 //  /// - If it is shorter and either the previous PhysicalCylinder has another daughter or the
 //  /// mother is not a PhysicalCylinder, it disappears.
 //  /// \param speed of the retraction (microns/hours).
 //  /// \return false if the neurite doesn't exist anymore (complete retraction)
 //  ///
 //  bool retractCylinder(double speed);
 //
 //  ///
 //  /// Bifurcation of the growth cone creating : adds the 2 <code>PhysicalCylinder</code> that become
 //  /// daughter left and daughter right
 //  /// \param length the length of the new branches
 //  /// \param direction_1 of the first branch (if
 //  /// \param newBranchL
 //  /// \param newBranchR
 //  ///
 //
 //  std::array<UPtr, 2> bifurcateCylinder(double length, const std::array<double, 3>& direction_1,
 //                                        const std::array<double, 3>& direction_2);
 //
 //  ///
 //  /// Makes a side branching by adding a second daughter to a non terminal <code>PhysicalCylinder</code>.
 //  /// The new <code>PhysicalCylinder</code> is perpendicular to the mother branch.
 //  /// \param direction the direction of the new neuriteLement (But will be automatically corrected if
 //  /// not al least 45 degrees from the cylinder's axis).
 //  /// \return the newly added <code>NeuriteSegment</code>
 //  ///
 //  UPtr branchCylinder(double length, const std::array<double, 3>& direction);
 //
 //  void setRestingLengthForDesiredTension(double tension);
 //
 //  ///
 //  /// Progressive modification of the volume. Updates the diameter, the intracellular concentration
 //  /// \param speed cubic micron/ h
 //  ///
 //  void changeVolume(double speed) override;
 //
 //  ///
 //  /// Progressive modification of the diameter. Updates the volume, the intracellular concentration
 //  /// \param speed micron/ h
 //  ///
 //  void changeDiameter(double speed) override;
 //
 //  // *************************************************************************************
 //  //   Physics
 //  // *************************************************************************************
 //
 //  void runPhysics() override;
 //
 //  std::array<double, 3> getForceOn(PhysicalSphere* s) override;
 //
 //  std::array<double, 4> getForceOn(PhysicalCylinder* c) override;
 //
 //  bool isInContactWithSphere(PhysicalSphere* s) override;
 //
 //  bool isInContactWithCylinder(PhysicalCylinder* c) override;
 //
 //  /// Returns the point on this cylinder's spring axis that is the closest to the point p.*/
 //  std::array<double, 3> closestPointTo(const std::array<double, 3>& p);
 //
 //  void runIntracellularDiffusion() override;
 //
 //  std::array<double, 3> getUnitNormalVector(const std::array<double, 3>& position) const override;
 //
 //  ///
 //  /// Defines the three orthonormal local axis so that a cylindrical coordinate system
 //  /// can be used. The xAxis is aligned with the springAxis. The two other are in the
 //  /// plane perpendicular to springAxis. This method to update the axis was suggested by
 //  /// Matt Coock. - Although not perfectly exact, it is accurate enough for us to use.
 //  ///
 //  void updateLocalCoordinateAxis();
 //
 //  /// Recomputes diameter after volume has changed.*/
 //  void updateDiameter() override;
 //
  /// Recomputes volume, after diameter has been change. And makes a call for
  /// recomputing then concentration of IntracellularSubstances.*/
  void UpdateVolume();
 //
 //  // *************************************************************************************
 //  //   Coordinates transform
 //  // *************************************************************************************
 //
 //  ///
 //  /// 3 systems of coordinates :
 //  ///
 //  /// Global :   cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0) and (0,0,1)
 //  ///        with origin at (0,0,0).
 //  /// Local :    defined by orthogonal axis xAxis (=vect proximal to distal end), yAxis and zAxis,
 //  ///        with origin at proximal end
 //  /// Polar :    cylindrical coordinates [h,theta,r] with
 //  ///        h = first local coord (along xAxis),
 //  ///        theta = angle from yAxis,
 //  ///        r euclidian distance from xAxis;
 //  ///        with origin at proximal end
 //  ///
 //  ///  Note: The methods below transform POSITIONS and not DIRECTIONS !!!
 //  ///
 //  /// G -> L
 //  /// L -> G
 //  ///
 //  /// L -> P
 //  /// P -> L
 //  ///
 //  /// G -> P = G -> L, then L -> P
 //  /// P -> P = P -> L, then L -> G
 //  ///
 //
 //  ///
 //  /// G -> L
 //  /// Returns the position in the local coordinate system (xAxis, yXis, zAxis)
 //  /// of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
 //  /// \param positionInGlobalCoord
 //  /// \return
 //  ///
 //  std::array<double, 3> transformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const override;
 //
 //  ///
 //  /// L -> G
 //  /// Returns the position in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
 //  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
 //  /// \param positionInLocalCoord
 //  /// \return
 //  ///
 //  std::array<double, 3> transformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const override;
 //
 //  ///
 //  ///  L -> P
 //  /// Returns the position in cylindrical coordinates (h,theta,r)
 //  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
 //  /// \param positionInLocalCoord
 //  /// \return
 //  ///
 //  std::array<double, 3> transformCoordinatesLocalToPolar(const std::array<double, 3>& position) const;
 //  ///
 //  /// P -> L
 //  /// Returns the position in the local coordinate system (xAxis, yXis, zAxis)
 //  /// of a point expressed in cylindrical coordinates (h,theta,r).
 //  /// \param positionInLocalCoord
 //  /// \return
 //  ///
 //  std::array<double, 3> transformCoordinatesPolarToLocal(const std::array<double, 3>& position) const;
 //
 //  /// P -> G :    P -> L, then L -> G///
 //  std::array<double, 3> transformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const override;
 //
 //  /// G -> L :    G -> L, then L -> P///
 //  std::array<double, 3> transformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const override;
 //
 //  // *************************************************************************************
 //  //   GETTERS & SETTERS
 //  // *************************************************************************************
 //
 //  /// Well, there is no field cellElement. We return neuriteElement.*/
 //  CellElement* getCellElement() const override;
 //
 //  ///
 //  /// \return the neuriteElement
 //  ///
 //  NeuriteElement* getNeuriteElement() const;
 //
 //  ///
 //  /// \param neuriteElement the neuriteElement to set
 //  ///
 //  void setNeuriteElement(NeuriteElement* neurite);
 //
 //  ///
 //  /// \return the daughterLeft
 //  ///
 //  PhysicalCylinder* getDaughterLeft() const;
 //
 //  ///
 //  /// \return the daughterRight
 //  ///
 //  PhysicalCylinder* getDaughterRight() const;
 //
 //  ///
 //  /// \return the mother
 //  ///
 //  PhysicalObject* getMother() const;
 //
 //  ///
 //  /// \param mother the mother to set
 //  ///
 //  void setMother(PhysicalObject* mother);
 //
 //  ///
 //  /// \param daughterLeft the daughterLeft to set
 //  ///
 //  void setDaughterLeft(PhysicalCylinder* daughter_left);
 //
 //  ///
 //  /// \param daughterRight the daughterRight to set
 //  ///
 //  void setDaughterRight(PhysicalCylinder* daughter_right);
 //
 //  ///
 //  /// \param branchOrder the branchOrder to set
 //  ///
 //  void setBranchOrder(int branch_order);
 //
 //  ///
 //  /// \return the branchOrder
 //  ///
 //  int getBranchOrder() const;
 //
 //  double getActualLength() const;
 //
 //  ///
 //  /// Should not be used, since the actual length depends on the geometry.
 //  /// \param actualLength
 //  ///
 //  void setActualLength(double actual_length);
 //
 //  double getRestingLength() const;
 //
 //  void setRestingLength(double resting_length);
 //
 //  std::array<double, 3> getSpringAxis() const;
 //
 //  void setSpringAxis(const std::array<double, 3>& axis);
 //
 //  double getSpringConstant() const;
 //
 //  void setSpringConstant(double spring_constant);
 //
 //  double getTension() const;
 //
 //  void setTension(double tension);
 //
 //  ///
 //  /// NOT A "REAL" GETTER
 //  /// Gets a vector of length 1, with the same direction as the SpringAxis.
 //  /// \return a normalized springAxis
 //  ///
 //  std::array<double, 3> getUnitaryAxisDirectionVector() const;
 //
 //  ///
 //  /// Should return yes if the PhysicalCylinder is considered a terminal branch.
 //  /// \return is it a terminal branch
 //  ///
 //  bool isTerminal() const;
 //
 //  ///
 //  /// Returns true if a bifurcation is physicaly possible. That is if the PhysicalCylinder
 //  /// has no daughter and the actual length is bigger than the minimum required.
 //  /// \return
 //  ///
 //  bool bifurcationPermitted() const;
 //
 //  ///
 //  /// Returns true if a side branch is physically possible. That is if this is not a terminal
 //  /// branch and if there is not already a second daughter.
 //  /// \return
 //  ///
 //  bool branchPermitted() const;
 //
 //  ///
 //  /// retuns the position of the proximal end, ie the massLocation minus the spring axis.
 //  /// Is mainly used for paint
 //  /// \return
 //  ///
 //  std::array<double, 3> proximalEnd() const;
 //
 //  ///
 //  /// retuns the position of the distal end, ie the massLocation coordinates (but not the
 //  /// actual massLocation array).
 //  /// Is mainly used for paint
 //  /// \return
 //  ///
 //  std::array<double, 3> distalEnd() const;
 //
 //  ///
 //  /// Returns the total (actual) length of all the cylinders (including the one in which this method is
 //  /// called) before the previous branching point. Used to decide if long enough to bifurcate or branch,
 //  /// independently of the discretization.
 //  /// \return
 //  ///
 //  double lengthToProximalBranchingPoint() const;
 //
 //  /// returns true because this object is a PhysicalCylinder///
 //  bool isAPhysicalCylinder() const override;
 //
 //  double getLength() const override;
 //
 //  double getInterObjectForceCoefficient() const override;
 //
 //  void setInterObjectForceCoefficient(double coefficient) override;
 //
 //  std::array<double, 3> getAxis() const override;
 //
 //  ///
 //  /// Updates the spring axis, the actual length, the tension and the volume.
 //  ///
 //  /// For tension, T = k*(aL-rL)/rL.  k = spring constant,
 //  /// rL = resting length, aL = actual length. (Note the division by rL.
 //  /// Otherwise we could have Cylinders with big aL and rL = 0).
 //  /// <p>
 //  /// This method also automatically calls the <code>resetComputationCenterPosition()</code>
 //  /// method at the end.
 //  ///
 //  void updateDependentPhysicalVariables() override;
 //
 // protected:
 //  ///
 //  /// Updates the concentration of substances, based on the volume of the object.
 //  /// Is usually called after change of the volume (and therefore we don't modify it here)
 //  ///
 //  void updateIntracellularConcentrations() override;
 //
 //  ///
 //  /// Repositioning of the SpatialNode location (usually a Delaunay vertex) at the barycenter of the cylinder.
 //  /// If it is already closer than a quarter of the diameter of the cylinder, it is not displaced.
 //  ///
 //  void updateSpatialOrganizationNodePosition();

 private:
   vec<std::array<double, 3>> position_ = {{0.0, 0.0, 0.0}};

  // TODO has local biology module
  // vec<bool> is_axon_ = {{false}};

  /// Parent node in the neuron tree structure can be a Neurite segment
  /// or cell body
  // vec<NeuriteMother<>> mother_;

  using TNeurite = typename TCompileTimeParam::TNeurite;
  using SimBackend = typename TCompileTimeParam::SimulationBackend;

  /// First child node in the neuron tree structure (can only be a Neurite
  /// segment)
  vec<SoPointer<Self<SimBackend>, SimBackend>> daughter_left_;
  /// Second child node in the neuron tree structure. (can only be a Neurite
  /// segment)
  vec<SoPointer<Self<SimBackend>, SimBackend>> daughter_right_;

  // /// number of branching points from here to the soma (root of the neuron tree-structure).*/
  // int branch_order_ = 0;
  //
  // /// The part of the inter-object force transmitted to the mother (parent node) -- c.f. runPhysics()///
  // std::array<double, 3> force_to_transmit_to_proximal_mass_ = std::array<double, 3> { 0, 0, 0 };

  /// from the attachment point to the mass location = position_
  /// (proximal -> distal).
  vec<std::array<double, 3>> spring_axis_ = {{ 0, 0, 0 }};

  /// Real length of the PhysicalCylinder (norm of the springAxis).///
  vec<double> actual_length_ = {Param::kNeuriteDefaultActualLength};

  /// Tension in the cylinder spring.*/
  vec<double> tension_ = {Param::kNeuriteDefaultTension};

  /// Spring constant per distance unit (springConstant restingLength  = "real" spring constant).///
  vec<double> spring_constant_ = {Param::kNeuriteDefaultSpringConstant};

  /// The length of the internal spring where tension would be zero.
  /// T = k*(A-R)/R --> R = k*A/(T+K)
  vec<double> resting_length_ = {spring_constant_[kIdx] * actual_length_[kIdx] / (tension_[kIdx] + spring_constant_[kIdx])};

  // /// Divides the PhysicalCylinder into two PhysicalCylinders of equal length. The one in which the method is called becomes the distal half.
  // /// A new PhysicalCylinder is instantiated and becomes the proximal part. All characteristics are transmitted.
  // /// A new Neurite element is also instantiated, and assigned to the new proximal PhysicalCylinder
  // NeuriteElement* insertProximalCylinder();
  //
  // /// Divides the PhysicalCylinder into two PhysicalCylinders (in fact, into two instances of the derived class).
  // /// The one in which the method is called becomes the distal half, and it's length is reduced.
  // /// A new PhysicalCylinder is instantiated and becomes the proximal part (=the mother). All characteristics are transmitted
  // /// \param distalPortion the fraction of the total old length devoted to the distal half (should be between 0 and 1).
  // NeuriteElement* insertProximalCylinder(double distal_portion);
  //
  // /// Merges two Cylinders together. The one in which the method is called phagocytes it's mother.
  // /// The CellElement of the PhysicalCylinder that is removed is also removed: it's removeYourself() method is called.
  // void removeProximalCylinder();
  //
  // /// Sets the scheduling flag onTheSchedulerListForPhysicalObjects to true
  // /// for me and for all my neighbors, relative, things I share a physicalBond with
  // void scheduleMeAndAllMyFriends();
  //
  // UPtr extendSideCylinder(double length, const std::array<double, 3>& direction);
};

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
// template <typename T, template <typename> class U>
// void NeuriteExt<T, U>::RetractTerminalEnd(double speed) {
//     // check if is a terminal branch
//     if (!daughter_left_.IsNullPtr()) {
//       return true;
//     }
//     // TODO : what if there are some physical Bonds ??
//     // scaling for integration step
//     speed *= Param::kSimulationTimeStep;
//
//     // if actual_length_ > length : retraction keeping the same tension
//     // (putting a limit on how short a branch can be is absolutely necessary
//     //  otherwise the tension might explode)
//     if (actual_length_[kIdx] > speed + 0.1) {
//       double new_actual_length = actual_length_[kIdx] - speed;
//       double factor = new_actual_length / actual_length_[kIdx];
//       actual_length_[kIdx] = new_actual_length;
//       //cf removeproximalCylinder()
//       resting_length_[kIdx] = spring_constant_[kIdx] * actual_length_[kIdx] / (tension_[kIdx] + spring_constant_[kIdx]);
//       spring_axis_[kIdx] = {factor * spring_axis_[kIdx][0], factor*spring_axis_[kIdx][1], factor*spring_axis_[kIdx][2]};
//
//
//       position_[kIdx] = Matrix::Add(mother_.GetPosition(), spring_axis_[kIdx]);
//       UpdateVolume();  // and update concentration of internal stuff.
//       // be sure i'll run my physics :
//       // TODO setOnTheSchedulerListForPhysicalObjects(true);
//       // if actual_length_ < length and mother is a PhysicalCylinder with no other daughter : merge with mother
//       // TODO replace with static_if: https://stackoverflow.com/questions/37242375/using-sfinae-how-to-avoid-has-no-member-named
//     } else if (mother_.IsNeurite() && !mother_.GetDaughterRight().IsNullPtr()) {
//       // TODO RemoveProximalCylinder();  // also updates volume_...
//       // be sure i'll run my physics :
//       // TODO setOnTheSchedulerListForPhysicalObjects(true);
//       RetractTerminalEnd(speed / Param::kSimulationTimeStep);
//       // if mother is cylinder with other daughter or is not a cylinder : disappear.
//     } else {
//       mother_->RemoveDaughter(this);
//       // TODO still_existing_ = false;
//       // TODO ecm_->removePhysicalCylinder(this);  // this method removes the SONode
//       // and the associated neuriteElement also disappears :
//       // TODO neurite_element_->removeYourself();
//       // TODO intracellularSubstances quantities
//       // (concentrations are solved in updateDependentPhysicalVariables():
//       // for (auto& el : intracellular_substances_) {
//       //   auto s = el.second.get();
//       //   mother_->modifyIntracellularQuantity(s->getId(), s->getQuantity() / Param::kSimulationTimeStep);
//       //   // (divide by time step because it is multiplied by it in the method)
//       // }
//       // TODO mother_->updateDependentPhysicalVariables();
//       // extra-safe : make sure you'll not be run :
//       // TODO setOnTheSchedulerListForPhysicalObjects(false);
//     }
// }

template <typename T, template <typename> class U>
void NeuriteExt<T, U>::UpdateVolume() {
  // volume_[kIdx] = Math::kPi / 4 * diameter_[kIdx] * diameter_[kIdx] * actual_length_[kIdx];
  // TODO updateIntracellularConcentrations();
}

// auto& pos = rm->ApplyOnElement(mother_, [](auto&& so, SoHandle) { return so.GetPosition(); });
// auto& pos = SOH_CALL(mother_, GetPosition());
// auto& pos = SOH_CALL(mother_, Divide(0.4, *cell));
// SOH_CALL(mother_, SetPosition({0, 1, 2}));
// auto& pos = mother_->GetPosition();
//
//
// void RemoveDaughter(SoHandle handle) {
//   auto rm = TResourceManager<>::Get();
//   rm->ApplyOnElement(handle, [](auto&& so, SoHandle) {
//     so.RemoveDaughter();
//   });
// }
//
// const std::array<double, 3>& GetPosition(SoHandle handle) {
//   auto rm = TResourceManager<>::Get();
//   return rm->ApplyOnElement(handle, [](auto&& so, SoHandle) -> const auto& {
//     return so.GetPosition();
//   });
// }
//
//
//
// void Foo() {
//   std::vector<Cell> cells;
//   cells[5].GetPosition();
//
//   SoaCell soa;
//   soa[5].GetPosition();
//   soa.GetPosition(0);
//
//   class CellExt {
//     const std::array<double, 3>& GetPosition(uint64_t idx) {
//       return position_[idx];
//     }
//   }
//
//   template <typename T>
//   class CellRef {
//     const uint64_t index_;
//     T* sim_objects_ = nullptr;
//
//     CellRef() : index_(0) {} // creates nullreference
//     CellRef(uint64_t idx, T* sim_objects) : index_(idx), sim_objects_(sim_objects) {}
//
//     bool IsNullRef() const { return sim_objects_ != nullptr; }
//
//     const std::array<double, 3>& GetPosition() {
//       return sim_objects_->GetPosition(index_);
//     }
//   };
//
//   // SOA
//   Ref<Self<Soa>> operator[](uint64_t idx) {
//     return CellRef(idx, this);
//   }
//
//   // Scalar
//   Ref<Self<Scalar> operator[](uint64_t idx) {
//     return CellRef(0, &data_[idx]);
//   }


}  // namespace bdm

#endif  // NEUROSCIENCE_NEURITE_H_
