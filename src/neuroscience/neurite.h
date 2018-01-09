#ifndef NEUROSCIENCE_NEURITE_H_
#define NEUROSCIENCE_NEURITE_H_

#include "math_util.h"
#include "matrix.h"
#include "param.h"
#include "random.h"
#include "simulation_object.h"
#include "simulation_object_util.h"
#include <typeinfo>  // TODO remove
#include <iostream>  // TODO remove

#include "TError.h"

namespace bdm {
namespace neuroscience {

/// The mother of a neurite can either be a neuron or a neurite.
/// Therefore, this class acts as an intermediate layer that forwards function
/// calls to the correct object.
/// @tparam TNeuronSoPtr   type of Neuron simulation object pointer
/// @tparam TNeuriteSoPtr  type of Neurite simulation object pointer.
template<typename TNeuronSoPtr, typename TNeuriteSoPtr>
class NeuronNeuriteAdapter {
public:
  template <typename T>
  NeuronNeuriteAdapter(T&& soptr, typename std::enable_if<is_same<T, TNeuronSoPtr>::value>::type* p = 0) {
    neuron_ptr_ = soptr;
  }

  template <typename T>
  NeuronNeuriteAdapter(T&& soptr, typename std::enable_if<is_same<T, TNeuriteSoPtr>::value>::type* p = 0) {
    neurite_ptr_ = soptr;
  }


  template <typename T>
  typename std::enable_if<is_same<T, TNeuronSoPtr>::value>::type
  Set(T&& soptr) {
    neuron_ptr_ = soptr;
  }

  template <typename T>
  typename std::enable_if<is_same<T, TNeuriteSoPtr>::value>::type
  Set(T&& soptr) {
    neurite_ptr_ = soptr;
  }

  const std::array<double, 3>& GetPosition() const {
    if (!neurite_ptr_.IsNullPtr()) {
      return neurite_ptr_.Get().GetPosition();
    }
    return neuron_ptr_.Get().GetPosition();
  }

  bool IsNeuron() const { return !neuron_ptr_.IsNullPtr(); }
  bool IsNeurite() const { return !neurite_ptr_.IsNullPtr(); }

  auto GetDaughterLeft() -> decltype(std::declval<TNeuriteSoPtr>().Get().GetDaughterLeft()) const {
    assert(IsNeurite() && "This function call is only allowed for a Neurite");
    return neurite_ptr_.Get().GetDaughterLeft();
  }

  auto GetDaughterRight() -> decltype(std::declval<TNeuriteSoPtr>().Get().GetDaughterRight()) const {
    assert(IsNeurite() && "This function call is only allowed for a Neurite");
    return neurite_ptr_.Get().GetDaughterRight();
  }

  auto GetRestingLength() -> decltype(std::declval<TNeuriteSoPtr>().Get().GetRestingLength()) const {
    assert(IsNeurite() && "This function call is only allowed for a Neurite");
    return neurite_ptr_.Get().GetRestingLength();
  }

  void UpdateDependentPhysicalVariables() {
    if (!neurite_ptr_.IsNullPtr()) {
      neurite_ptr_.Get().UpdateDependentPhysicalVariables();
    }
    // FIXME
    //neuron_ptr_.Get().UpdateDependentPhysicalVariables();
  }

  auto RemoveYourself() -> decltype(std::declval<TNeuriteSoPtr>().Get().RemoveYourself()) const {
    assert(IsNeurite() && "This function call is only allowed for a Neurite");
    return neurite_ptr_.Get().RemoveYourself();
  }

  // TODO remove template param and change to TNeuriteSoPtr?
  void RemoveDaughter(const TNeuriteSoPtr& mother) {
    assert(IsNeuron() && "This function call is only allowed for a Neuron");
    neuron_ptr_.Get().RemoveDaughter(mother);
  }

private:
  TNeuronSoPtr neuron_ptr_;
  TNeuriteSoPtr neurite_ptr_;
};

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
// TODO
BDM_SIM_OBJECT(Neurite, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(NeuriteExt, 1, position_, volume_, diameter_, x_axis_, y_axis_, z_axis_,
                                  is_axon_,
                                  mother_, daughter_left_, daughter_right_, branch_order_, force_to_transmit_to_proximal_mass_, spring_axis_, actual_length_, tension_, spring_constant_, resting_length_);

  using TNeuron = typename TCompileTimeParam::TNeuron;

  // using NeuriteMother = NeuronNeuriteAdapter<Convert<TNeuron, SimBackend>, MostDerivedSB, SimBackend>;
  using NeuriteMother = NeuronNeuriteAdapter<ToSoPtr<TNeuron>, MostDerivedSoPtr>;

  // SoPointer<typename ToBackend<TNeuron, Scalar>::type, SimBackend> aa_;
  ToSoPtr<TNeuron> aa_;

 public:
   NeuriteExt() {}
   const std::array<double, 3>& GetPosition() const { return position_[kIdx]; }
   void SetDiameter(double diameter) {
     std::cout << kIdx << " - " << diameter << std::endl;
    //  diameter_[kIdx] = diameter;
   }
   void SetPosition(const std::array<double, 3>& position) { position_[kIdx] = position; }

  /// Retracts the cylinder, if it is a terminal one.
  /// Branch retraction by moving the distal end toward the
  /// proximal end (the mother), maintaining the same tension in the PhysicalCylinder. The method
  /// shortens the actual and the resting length so that the result is a shorter
  /// cylinder with the same tension.
  ///   * If this neurite element is longer than the required shortening, it simply retracts.
  ///   * If it is shorter and its mother has no other daughter, it merges with it's mother and
  /// the method is recursively called (this time the cylinder length is bigger because we have
  /// a new neurite element that resulted from the fusion of two).
  ///   * If it is shorter and either the previous neurite element has another daughter or the
  /// mother is not a neurite element, it disappears.
  /// @param speed the retraction speed in microns / h
  void RetractTerminalEnd(double speed);

  /// Method used for active extension of a terminal branch, representing the steering of a
  /// growth cone. The movement should always be forward, otherwise no movement is performed.
  /// If `direction` points in an opposite direction than the axis, i.e.
  /// if the dot product is negative, there is no movement (only elongation is possible).
  /// @param speed
  /// @param direction
  void ElongateTerminalEnd(double speed, const std::array<double, 3>& direction);

  /// Returns true if a side branch is physically possible. That is if this is not a terminal
  /// branch and if there is not already a second daughter.
  bool BranchPermitted() const;

  /// Makes a side branch, i.e. splits this neurite element into two and puts a daughter right at the proximal half.
  /// @param new_branch_diameter
  /// @param direction growth direction, but will be automatically corrected if not at least 45 degrees from the cylinder's axis.
  TMostDerived<Scalar> Branch(double new_branch_diameter, const std::array<double, 3>& direction);

  /// Makes a side branch, i.e. splits this neurite element into two and puts a daughter right at the proximal half.
  /// @param direction growth direction, but will be automatically corrected if not at least 45 degrees from the cylinder's axis.
  TMostDerived<Scalar> Branch(const std::array<double, 3>& direction);

  /// Makes a side branch, i.e. splits this neurite element into two and puts a daughter right at the proximal half.
  /// @param diameter of the side branch
  TMostDerived<Scalar> Branch(double diameter);

  /// Makes a side branch, i.e. splits this neurite element into two and puts a daughter right at the proximal half.
  TMostDerived<Scalar> Branch();

  /// Returns true if a bifurcation is physicaly possible. That is if the neurite element
  /// has no daughter and the actual length is bigger than the minimum required.
  bool BifurcationPermitted() const;

 //  ///
 //  /// Bifurcation of a growth come (only works for terminal segments).
 //  /// Note : angles are corrected if they are pointing backward.
 //  /// @param diameter_1  of new daughterLeft
 //  /// @param diameter_2 of new daughterRight
 //  /// @param direction_1
 //  /// @param direction_2
 //  /// \return
 //  ///
 //  std::array<NeuriteElement*, 2> bifurcate(double diameter_1, double diameter_2,
 //                                           const std::array<double, 3>& direction_1,
 //                                           const std::array<double, 3>& direction_2);
 //
 //  /// Bifurcation of a growth come (only works for terminal segments).
 //  /// Note : angles are corrected if they are pointing backward.
 //  /// @param length of new branches
 //  /// @param diameter_1  of new daughterLeft
 //  /// @param diameter_2 of new daughterRight
 //  /// @param direction_1
 //  /// @param direction_2
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


  // *************************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE                                            *
  // *************************************************************************************

  void RemoveDaughter(const MostDerivedSoPtr& daughter);

  void UpdateRelative(const MostDerivedSoPtr& old_relative,
                      const MostDerivedSoPtr& new_relative);

  /// Returns the total force that this `Neurite` exerts on it's mother.
  /// It is the sum of the spring force and the part of the inter-object force
  /// computed earlier in `RunPhyiscs`
  // TODO update Documentation (RunPhysics is probably displacement op)
  std::array<double, 3> ForceTransmittedFromDaugtherToMother(const NeuriteMother& mother);

  // *************************************************************************************
  //   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
  // *************************************************************************************

  /// Checks if this Neurite is either too long or too short.
  ///   * too long: insert another Neurite
  ///   * too short fuse it with the proximal element or even delete it
  void RunDiscretization();

  // *************************************************************************************
  //   ELONGATION, RETRACTION, BRANCHING
  // *************************************************************************************

  /// Method used for active extension of a terminal branch, representing the steering of a
  /// growth cone. There is no check for real extension (unlike in `ExtendCylinder()`` ).
  ///
  /// @param speed      of the growth rate (microns/hours).
  /// @param direction  the 3D direction of movement.
  ///
  void MovePointMass(double speed, const std::array<double, 3>& direction);

 //
 //  ///
 //  /// Bifurcation of the growth cone creating : adds the 2 <code>PhysicalCylinder</code> that become
 //  /// daughter left and daughter right
 //  /// @param length the length of the new branches
 //  /// @param direction_1 of the first branch (if
 //  /// @param newBranchL
 //  /// @param newBranchR
 //  ///
 //
 //  std::array<UPtr, 2> bifurcateCylinder(double length, const std::array<double, 3>& direction_1,
 //                                        const std::array<double, 3>& direction_2);
 //
 //  ///
 //  /// Makes a side branching by adding a second daughter to a non terminal <code>PhysicalCylinder</code>.
 //  /// The new <code>PhysicalCylinder</code> is perpendicular to the mother branch.
 //  /// @param direction the direction of the new neuriteLement (But will be automatically corrected if
 //  /// not al least 45 degrees from the cylinder's axis).
 //  /// \return the newly added <code>NeuriteSegment</code>
 //  ///
 //  UPtr branchCylinder(double length, const std::array<double, 3>& direction);
 //
  void SetRestingLengthForDesiredTension(double tension);

  /// Progressive modification of the volume. Updates the diameter, the intracellular concentration
  /// @param speed cubic micron/ h
  void ChangeVolume(double speed);

  /// Progressive modification of the diameter. Updates the volume, the intracellular concentration
  /// @param speed micron/ h
  void ChangeDiameter(double speed);
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
  /// Defines the three orthonormal local axis so that a cylindrical coordinate system
  /// can be used. The xAxis is aligned with the `spring_axis_`. The two other are in the
  /// plane perpendicular to `spring_axis_`. This method to update the axis was suggested by
  /// Matt Coock. - Although not perfectly exact, it is accurate enough for us to use.
  void UpdateLocalCoordinateAxis();

  /// Recomputes diameter after volume has changed.*/
  void UpdateDiameter();

  /// Recomputes volume, after diameter has been change. And makes a call for
  /// recomputing then concentration of IntracellularSubstances.*/
  void UpdateVolume();

  // *************************************************************************************
  //   Coordinates transform
  // *************************************************************************************

  /// 3 systems of coordinates :
  ///
  /// Global :   cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0) and (0,0,1)
  ///        with origin at (0,0,0).
  /// Local :    defined by orthogonal axis xAxis (=vect proximal to distal end), yAxis and zAxis,
  ///        with origin at proximal end
  /// Polar :    cylindrical coordinates [h,theta,r] with
  ///        h = first local coord (along xAxis),
  ///        theta = angle from yAxis,
  ///        r euclidian distance from xAxis;
  ///        with origin at proximal end
  ///
  ///  Note: The methods below transform POSITIONS and not DIRECTIONS !!!
  ///
  /// G -> L
  /// L -> G
  ///
  /// L -> P
  /// P -> L
  ///
  /// G -> P = G -> L, then L -> P
  /// P -> P = P -> L, then L -> G

  /// G -> L
  /// Returns the position in the local coordinate system (xAxis, yXis, zAxis)
  /// of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
  /// @param position in global coordinates
  std::array<double, 3> TransformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const;

  /// L -> G
  /// Returns the position in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
  /// @param position in local coordinates
  std::array<double, 3> TransformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const;

  ///  L -> P
  /// Returns the position in cylindrical coordinates (h,theta,r)
  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
  /// @param position in local coordinates
  std::array<double, 3> TransformCoordinatesLocalToPolar(const std::array<double, 3>& position) const;

  /// P -> L
  /// Returns the position in the local coordinate system (xAxis, yXis, zAxis)
  /// of a point expressed in cylindrical coordinates (h,theta,r).
  /// @param position in local coordinates
  std::array<double, 3> TransformCoordinatesPolarToLocal(const std::array<double, 3>& position) const;

  /// P -> G :    P -> L, then L -> G
  std::array<double, 3> TransformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const;

  /// G -> L :    G -> L, then L -> P
  std::array<double, 3> TransformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const;

  // *************************************************************************************
  //   GETTERS & SETTERS
  // *************************************************************************************

  bool IsAxon() const;

  void SetAxon(bool is_axon);

  const NeuriteMother& GetMother() const;

  void SetMother(const NeuriteMother& mother);

  /// @return the (first) distal neurite element, if it exists,
  /// i.e. if this is not the terminal segment (otherwise returns nullptr).
  const MostDerivedSoPtr& GetDaughterLeft() const;

  void SetDaughterLeft(const MostDerivedSoPtr& daughter);

  /// @return the second distal neurite element, if it exists
  /// i.e. if there is a branching point just after this element (otherwise returns nullptr).
  const MostDerivedSoPtr& GetDaughterRight() const;

  void SetDaughterRight(const MostDerivedSoPtr& daughter);

  int GetBranchOrder() const;

  void SetBranchOrder(int branch_order);

  double GetActualLength() const;

  /// Should not be used, since the actual length depends on the geometry.
  void SetActualLength(double actual_length);

  double GetRestingLength() const;

  void SetRestingLength(double resting_length);

  const std::array<double, 3>& GetSpringAxis() const;

  void SetSpringAxis(const std::array<double, 3>& axis);

  double GetSpringConstant() const;

  void SetSpringConstant(double spring_constant);

  double GetTension() const;

  void SetTension(double tension);

  /// NOT A "REAL" GETTER
  /// Gets a vector of length 1, with the same direction as the SpringAxis.
  /// @return a normalized spring axis
  std::array<double, 3> GetUnitaryAxisDirectionVector() const;

  /// Should return yes if the PhysicalCylinder is considered a terminal branch.
  /// @return is it a terminal branch
  bool IsTerminal() const;

  /// retuns the position of the proximal end, ie the position minus the spring axis.
  /// Is mainly used for paint
  std::array<double, 3> ProximalEnd() const;

  /// Returns the position of the distal end == position_
  const std::array<double, 3>& DistalEnd() const;

  /// Returns the total (actual) length of all the neurite elements (including the one in which this method is
  /// called) before the previous branching point. Used to decide if long enough to bifurcate or branch,
  /// independently of the discretization.
  double LengthToProximalBranchingPoint() const;

  double GetLength() const;

  /// Returns the axis direction of a neurite element
  const std::array<double, 3>& GetAxis() const;

  /// Updates the spring axis, the actual length, the tension and the volume.
  ///
  /// For tension, T = k*(aL-rL)/rL.  k = spring constant,
  /// rL = resting length, aL = actual length. (Note the division by rL.
  /// Otherwise we could have cylinders with big aL and rL = 0).\n
  /// This method also automatically calls the <code>resetComputationCenterPosition()</code>
  /// method at the end.
  void UpdateDependentPhysicalVariables();
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

  void RemoveYourself();

 private:

   // TODO data members same as in cell
   /// position_ is distal end of the cylinder
   vec<std::array<double, 3>> position_ = {{0.0, 0.0, 0.0}};
   vec<double> volume_;
   vec<double> diameter_ = {{ Param::kNeuriteDefaultDiameter }};
   /// First axis of the local coordinate system equal to cylinder axis
   vec<array<double, 3>> x_axis_ = {{1.0, 0.0, 0.0}};
   /// Second axis of the local coordinate system.
   vec<array<double, 3>> y_axis_ = {{0.0, 1.0, 0.0}};
   /// Third axis of the local coordinate system.
   vec<array<double, 3>> z_axis_ = {{0.0, 0.0, 1.0}};


  vec<bool> is_axon_ = {{false}};

  /// Parent node in the neuron tree structure can be a Neurite segment
  /// or cell body
  vec<NeuriteMother> mother_ = {{}};

  /// First child node in the neuron tree structure (can only be a Neurite
  /// segment)
  vec<MostDerivedSoPtr> daughter_left_;
  /// Second child node in the neuron tree structure. (can only be a Neurite
  /// segment)
  vec<MostDerivedSoPtr> daughter_right_;

  /// number of branching points from here to the soma (root of the neuron tree-structure).*/
  vec<int> branch_order_  = { 0 };

  /// The part of the inter-object force transmitted to the mother (parent node)
  vec<std::array<double, 3>> force_to_transmit_to_proximal_mass_ = {{ 0, 0, 0 }};

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

  // TODO documentation TODO rename function
  /// Divides the PhysicalCylinder into two PhysicalCylinders of equal length. The one in which the method is called becomes the distal half.
  /// A new PhysicalCylinder is instantiated and becomes the proximal part. All characteristics are transmitted.
  /// A new Neurite element is also instantiated, and assigned to the new proximal PhysicalCylinder
  void InsertProximalCylinder(TMostDerived<Scalar>* new_neurite_element);

  // TODO documentation, TODO rename function
  /// Divides the PhysicalCylinder into two PhysicalCylinders (in fact, into two instances of the derived class).
  /// The one in which the method is called becomes the distal half, and it's length is reduced.
  /// A new PhysicalCylinder is instantiated and becomes the proximal part (=the mother). All characteristics are transmitted
  /// @param distalPortion the fraction of the total old length devoted to the distal half (should be between 0 and 1).
  void InsertProximalCylinder(TMostDerived<Scalar>* new_neurite_element, double distal_portion);

  // TODO rename function
  /// Merges two neurite elements together. The one in which the method is called phagocytes it's mother.
  void RemoveProximalCylinder();

  //
  // /// Sets the scheduling flag onTheSchedulerListForPhysicalObjects to true
  // /// for me and for all my neighbors, relative, things I share a physicalBond with
  // void scheduleMeAndAllMyFriends();

  // TODO rename function
  void ExtendSideCylinder(TMostDerived<Scalar>* new_branch, double length, const std::array<double, 3>& direction);
};

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
BDM_SO_DEFINE(inline void NeuriteExt)::RetractTerminalEnd(double speed) {
    // check if is a terminal branch
    if (!daughter_left_[kIdx].IsNullPtr()) {
      return;
    }
    // TODO : what if there are some physical Bonds ??
    // scaling for integration step
    speed *= Param::simulation_time_step_;

    if (actual_length_[kIdx] > speed + 0.1) {
      // if actual_length_ > length : retraction keeping the same tension
      // (putting a limit on how short a branch can be is absolutely necessary
      //  otherwise the tension might explode)

      double new_actual_length = actual_length_[kIdx] - speed;
      double factor = new_actual_length / actual_length_[kIdx];
      actual_length_[kIdx] = new_actual_length;
      //cf removeproximalCylinder()
      resting_length_[kIdx] = spring_constant_[kIdx] * actual_length_[kIdx] / (tension_[kIdx] + spring_constant_[kIdx]);
      spring_axis_[kIdx] = {factor * spring_axis_[kIdx][0], factor*spring_axis_[kIdx][1], factor*spring_axis_[kIdx][2]};

      position_[kIdx] = Matrix::Add(mother_[kIdx].GetPosition(), spring_axis_[kIdx]);
      UpdateVolume();  // and update concentration of internal stuff.
      // be sure i'll run my physics :
      // TODO setOnTheSchedulerListForPhysicalObjects(true);
    } else if (mother_[kIdx].IsNeurite() && !mother_[kIdx].GetDaughterRight().IsNullPtr()) {
      // if actual_length_ < length and mother is a PhysicalCylinder with no other daughter : merge with mother
      // TODO RemoveProximalCylinder();  // also updates volume_...
      // be sure i'll run my physics :
      // TODO setOnTheSchedulerListForPhysicalObjects(true);
      RetractTerminalEnd(speed / Param::simulation_time_step_);
    } else {
      // if mother is cylinder with other daughter or is not a cylinder : disappear.
      mother_[kIdx].RemoveDaughter(GetSoPtr());
      // TODO still_existing_ = false;
      // TODO ecm_->removePhysicalCylinder(this);  // this method removes the SONode
      // and the associated neuriteElement also disappears :
      // TODO neurite_element_->removeYourself();
      // TODO intracellularSubstances quantities
      // (concentrations are solved in updateDependentPhysicalVariables():
      // for (auto& el : intracellular_substances_) {
      //   auto s = el.second.get();
      //   mother_->modifyIntracellularQuantity(s->getId(), s->getQuantity() / Param::simulation_time_step_);
      //   // (divide by time step because it is multiplied by it in the method)
      // }
      mother_[kIdx].UpdateDependentPhysicalVariables();
      // extra-safe : make sure you'll not be run :
      // TODO setOnTheSchedulerListForPhysicalObjects(false);
    }
}

BDM_SO_DEFINE(inline void NeuriteExt)::ElongateTerminalEnd(double speed, const std::array<double, 3>& direction) {
  double temp = Matrix::Dot(direction, spring_axis_[kIdx]);
  if (temp > 0) {
    MovePointMass(speed, direction);
  }
}

BDM_SO_DEFINE(inline bool NeuriteExt)::BranchPermitted() const {
  return !daughter_left_[kIdx].IsNullPtr() && daughter_right_.IsNullPtr();
}

// BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::template TMostDerived<Scalar> NeuriteExt)::Branch(double new_branch_diameter, const std::array<double, 3>& direction) {
//   // create a new neurite element for side branch
//   TMostDerived<Scalar> new_branch;
//   // TODO auto new_neurite = getCopy();
//
//   // TODO fixme
//   // we first split this cylinder into two pieces
//   // InsertProximalCylinder(&new_branch);
//   // then append a "daughter right" between the two
//   // return ne->getPhysicalCylinder()->extendSideCylinder(length, direction);
//
//   // making the branching at physicalObject level
//   // auto TODO pc_1 = physical_cylinder_->branchCylinder(1.0, direction);
//   new_branch.SetDiameter(diameter_[kIdx]);
//   new_branch.SetBranchOrder(branch_order_[kIdx] + 1);
//   // TODO : Caution : doesn't change the value distally on the main branch
//
//   // TODO
//   // Copy of the local biological modules:
//   // for (auto m : getLocalBiologyModulesList()) {
//   //   if (m->isCopiedWhenNeuriteBranches()) {
//   //     ne->addLocalBiologyModule(m->getCopy());
//   //   }
//   // }
//   return ne;
// }

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::template TMostDerived<Scalar> NeuriteExt)::Branch(const std::array<double, 3>& direction) {
  return Branch(diameter_[kIdx], direction);
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::template TMostDerived<Scalar> NeuriteExt)::Branch(double diameter) {
  auto rand_noise = gRandom.NextNoise(0.1);
  auto growth_direction = Math::Perp3(Matrix::Add(GetUnitaryAxisDirectionVector(), rand_noise),
                                        gRandom.NextDouble());
  growth_direction = Math::Normalize(growth_direction);
  return Branch(diameter, growth_direction);
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::template TMostDerived<Scalar> NeuriteExt)::Branch() {
  double branch_diameter = diameter_[kIdx];
  auto rand_noise = gRandom.NextNoise(0.1);
  auto growth_direction = Math::Perp3(Matrix::Add(GetUnitaryAxisDirectionVector(), rand_noise),
                                        gRandom.NextDouble());
  return Branch(branch_diameter, growth_direction);
}

BDM_SO_DEFINE(inline bool NeuriteExt)::BifurcationPermitted() const {
  return (daughter_left_[kIdx].IsNullPtr() && actual_length_[kIdx] > Param::kNeuriteMinimalBifurcationLength);
}

BDM_SO_DEFINE(inline void NeuriteExt)::RemoveDaughter(const typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr& daughter) {
  // If there is another daughter than the one we want to remove,
  // we have to be sure that it will be the daughterLeft->
  if (daughter == daughter_right_[kIdx]) {
    daughter_right_[kIdx] = nullptr;
    return;
  }

  if (daughter == daughter_left_[kIdx]) {
    daughter_left_[kIdx] = daughter_right_[kIdx];
    daughter_right_[kIdx] = nullptr;
    return;
  }
  Fatal("Neurite", "Given object is not a daughter!");
}

BDM_SO_DEFINE(inline void NeuriteExt)::UpdateRelative(const MostDerivedSoPtr& old_relative,
                                                      const MostDerivedSoPtr& new_relative) {
  if (old_relative == mother_[kIdx]) {
    mother_[kIdx] = new_relative;
  } else  if (old_relative == daughter_left_[kIdx]) {
    daughter_left_[kIdx] == new_relative;
  } else if (old_relative == daughter_right_[kIdx]) {
    daughter_right_[kIdx] == new_relative;
  }
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::ForceTransmittedFromDaugtherToMother(const NeuriteMother& mother) {
  if(mother_[kIdx] != mother) {
    Fatal("Neurite", "Given object is not the mother!");
  }

  // The inner tension is added to the external force that was computed earlier.
  // (The reason for dividing by the actualLength is to normalize the direction : T = T * axis/ (axis length)
  double factor = tension_[kIdx] / actual_length_[kIdx];
  if (factor < 0) {
    factor = 0;
  }
  // TODO
  // return {
  //   factor*spring_axis_[0] + force_to_transmit_to_proximal_mass_[0],
  //   factor*spring_axis_[1] + force_to_transmit_to_proximal_mass_[1],
  //   factor*spring_axis_[2] + force_to_transmit_to_proximal_mass_[2]
  // };
  return Matrix::Add(Matrix::ScalarMult(factor, spring_axis_[kIdx]), force_to_transmit_to_proximal_mass_[kIdx]);
}

BDM_SO_DEFINE(inline void NeuriteExt)::RunDiscretization() {
  if (actual_length_[kIdx] > Param::kNeuriteMaxLength) {
    if (daughter_left_[kIdx].IsNullPtr()) { // if terminal branch :
      // TODO InsertProximalCylinder(0.1);
    } else if (mother_[kIdx].IsNeuron()) {  // if initial branch :
      // TODO InsertProximalCylinder(0.9);
    } else {
      // TODO InsertProximalCylinder(0.5);
    }
  } else if (actual_length_[kIdx] < Param::kNeuriteMinLength && mother_[kIdx].IsNeurite()
      && mother_[kIdx].GetRestingLength() < Param::kNeuriteMaxLength - resting_length_[kIdx] - 1
      && mother_[kIdx].GetDaughterRight().IsNullPtr() && !daughter_left_[kIdx].IsNullPtr()) {
    // if the previous branch is removed, we first remove its associated NeuriteElement
    mother_[kIdx].RemoveYourself();
    // then we remove it
    // TODO RemoveProximalCylinder();
  }
}

BDM_SO_DEFINE(inline void NeuriteExt)::MovePointMass(double speed, const std::array<double, 3>& direction) {
  // check if is a terminal branch
  if (!daughter_left_[kIdx].IsNullPtr()) {
    return;
  }

  // scaling for integration step
  double length = speed * Param::simulation_time_step_;
  // auto normalized_dir = Math::Normalize(direction);
  // std::array<double, 3> displacement { length * normalized_dir[0], length * normalized_dir[1], length
  //     * normalized_dir[2] };
  auto&& displacement = Matrix::ScalarMult(length, Math::Normalize(direction));
  // mass_location_ = Matrix::add(displacement, mass_location_);
  position_[kIdx] = Matrix::Add(displacement, position_[kIdx]);
  // here I have to define the actual length ..........
  auto& relative_ml = mother_[kIdx].GetPosition();
  spring_axis_[kIdx] = Matrix::Subtract(position_[kIdx], relative_ml);
  // actual_length_ = MathUtil::sqrt(
  //     spring_axis_[0] * spring_axis_[0] + spring_axis_[1] * spring_axis_[1] + spring_axis_[2] * spring_axis_[2]);
  actual_length_[kIdx] = std::sqrt(Matrix::Dot(spring_axis_[kIdx], spring_axis_[kIdx]));
  // process of elongation : setting tension to 0 increases the restingLength :
  SetRestingLengthForDesiredTension(0.0);

  // some physics and computation obligations....
  UpdateVolume();  // and update concentration of internal stuff.
  UpdateLocalCoordinateAxis();
  // updateSpatialOrganizationNodePosition();
  // Make sure I'll be updated when I run my physics
  // but since I am actually moving, I have to update the neighbors
  // (the relative would probably not be needed...).
  // TODO scheduleMeAndAllMyFriends();
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetRestingLengthForDesiredTension(double tension) {
  tension_[kIdx] = tension;
  // T = k*(A-R)/R --> R = k*A/(T+K)
  resting_length_[kIdx] = spring_constant_[kIdx] * actual_length_[kIdx] / (tension_[kIdx] + spring_constant_[kIdx]);
}

BDM_SO_DEFINE(inline void NeuriteExt)::ChangeVolume(double speed) {
  //scaling for integration step
  double dV = speed * Param::simulation_time_step_;
  volume_[kIdx] += dV;

  if (volume_[kIdx] < 5.2359877E-7) {  // minimum volume_, corresponds to minimal diameter_
    volume_[kIdx] = 5.2359877E-7;
  }
  UpdateDiameter();
  // TODO updateIntracellularConcentrations();
  // TODO scheduleMeAndAllMyFriends();
}

BDM_SO_DEFINE(inline void NeuriteExt)::ChangeDiameter(double speed) {
  //scaling for integration step
  double dD = speed * Param::simulation_time_step_;
  diameter_[kIdx] += dD;
  UpdateVolume();
  // no call to updateIntracellularConcentrations() cause it's done by updateVolume().
  // TODO scheduleMeAndAllMyFriends();
}

BDM_SO_DEFINE(inline void NeuriteExt)::UpdateLocalCoordinateAxis() {
  // x (new) = something new
  // z (new) = x (new) cross y(old)
  // y (new) = z(new) cross x(new)
  x_axis_[kIdx] = Math::Normalize(spring_axis_[kIdx]);
  z_axis_[kIdx] = Math::CrossProduct(x_axis_[kIdx], y_axis_[kIdx]);
  double norm_of_z = Math::Norm(z_axis_[kIdx]);
  if (norm_of_z < 1E-10) { // TODO use parameter
    // If new x_axis_ and old y_axis_ are aligned, we cannot use this scheme;
    // we start by re-defining new perp vectors. Ok, we loose the previous info, but
    // this should almost never happen....
    z_axis_[kIdx] = Math::Perp3(x_axis_[kIdx], gRandom.NextDouble());
  } else {
    z_axis_[kIdx] = Matrix::ScalarMult((1 / norm_of_z), z_axis_[kIdx]);
  }
  y_axis_[kIdx] = Math::CrossProduct(z_axis_[kIdx], x_axis_[kIdx]);
}

BDM_SO_DEFINE(inline void NeuriteExt)::UpdateDiameter() {
  diameter_[kIdx] = std::sqrt(4 / Math::kPi * volume_[kIdx] / actual_length_[kIdx]);
}

BDM_SO_DEFINE(inline void NeuriteExt)::UpdateVolume() {
  volume_[kIdx] = Math::kPi / 4 * diameter_[kIdx] * diameter_[kIdx] * actual_length_[kIdx];
  // TODO updateIntracellularConcentrations();
}



BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::TransformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const {
  auto pos = Matrix::Subtract(position, ProximalEnd());
  return {
    Matrix::Dot(pos,x_axis_[kIdx]),
    Matrix::Dot(pos,y_axis_[kIdx]),
    Matrix::Dot(pos,z_axis_[kIdx])
  };
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::TransformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const {
  std::array<double, 3> glob { position[0] * x_axis_[kIdx][0] + position[1] * y_axis_[kIdx][0] + position[2] * z_axis_[kIdx][0],
      position[0] * x_axis_[kIdx][1] + position[1] * y_axis_[kIdx][1] + position[2] * z_axis_[kIdx][1], position[0] * x_axis_[kIdx][2]
          + position[1] * y_axis_[kIdx][2] + position[2] * z_axis_[kIdx][2] };
  return Matrix::Add(glob, ProximalEnd());
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::TransformCoordinatesLocalToPolar(const std::array<double, 3>& position) const {
  return {
    position[0],
    std::atan2(position[2], position[1]),
    std::sqrt(position[1]*position[1] + position[2]*position[2])
  };
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::TransformCoordinatesPolarToLocal(const std::array<double, 3>& position) const {
  return {
    position[0],
    position[2]*std::cos(position[1]),
    position[2]*std::sin(position[1])
  };
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::TransformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const {
  // the position is in cylindrical coord (h,theta,r)
  // with r being implicit (half the diameter_)
  // We thus have h (along x_axis_) and theta (the angle from the y_axis_).
  double r = 0.5 * diameter_;
  std::array<double, 3> polar_position { position[0], position[1], r };
  auto local = TransformCoordinatesPolarToLocal(polar_position);
  return TransformCoordinatesLocalToGlobal(local);
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::TransformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const {
  auto local = TransformCoordinatesGlobalToLocal(position);
  return TransformCoordinatesLocalToPolar(local);
}

BDM_SO_DEFINE(inline bool NeuriteExt)::IsAxon() const {
  return is_axon_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetAxon(bool is_axon) {
  is_axon_[kIdx] = is_axon;
}

BDM_SO_DEFINE(inline const typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::NeuriteMother& NeuriteExt)::GetMother() const {
  return mother_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetMother(const NeuriteMother& mother) {
  mother_[kIdx] = mother;
}

BDM_SO_DEFINE(inline const typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr& NeuriteExt)::GetDaughterLeft() const {
  return daughter_left_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetDaughterLeft(const MostDerivedSoPtr& daughter) {
  daughter_left_[kIdx] = daughter;
}

BDM_SO_DEFINE(inline const typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr& NeuriteExt)::GetDaughterRight() const {
  return daughter_right_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetDaughterRight(const MostDerivedSoPtr& daughter){
  daughter_right_[kIdx] = daughter;
}

BDM_SO_DEFINE(inline int NeuriteExt)::GetBranchOrder() const {
  return branch_order_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetBranchOrder(int branch_order) {
  branch_order_[kIdx] = branch_order;
}

BDM_SO_DEFINE(inline double NeuriteExt)::GetActualLength() const {
  return actual_length_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetActualLength(double actual_length) {
  actual_length_[kIdx] = actual_length;
}

BDM_SO_DEFINE(inline double NeuriteExt)::GetRestingLength() const {
  return resting_length_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetRestingLength(double resting_length) {
  resting_length_[kIdx] = resting_length;
}

BDM_SO_DEFINE(inline const std::array<double, 3>& NeuriteExt)::GetSpringAxis() const {
  return spring_axis_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetSpringAxis(const std::array<double, 3>& axis) {
  spring_axis_[kIdx] = axis;
}

BDM_SO_DEFINE(inline double NeuriteExt)::GetSpringConstant() const {
  return spring_constant_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetSpringConstant(double spring_constant) {
  spring_constant_[kIdx] = spring_constant;
}

BDM_SO_DEFINE(inline double NeuriteExt)::GetTension() const {
  return tension_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetTension(double tension) {
  tension_[kIdx] = tension;
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::GetUnitaryAxisDirectionVector() const {
  double factor = 1.0 / actual_length_;
  // return {factor*spring_axis_[kIdx][0], factor*spring_axis_[kIdx][1], factor*spring_axis_[kIdx][2]};
  return Matrix::ScalarMult(factor, spring_axis_[kIdx]);
}

BDM_SO_DEFINE(inline bool NeuriteExt)::IsTerminal() const {
  return daughter_left_[kIdx].IsNullPtr();
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::ProximalEnd() const {
  return Matrix::Subtract(position_[kIdx], spring_axis_[kIdx]);
}

BDM_SO_DEFINE(inline const std::array<double, 3>& NeuriteExt)::DistalEnd() const {
  return position_[kIdx];
}

BDM_SO_DEFINE(inline double NeuriteExt)::LengthToProximalBranchingPoint() const {
  double length = actual_length_;
  if (mother_->IsNeurite()) {
    if (mother_->GetDaughterRight().IsNullPtr()) {
      length += mother_->LengthToProximalBranchingPoint();
    }
  }
  return length;
}

BDM_SO_DEFINE(inline double NeuriteExt)::GetLength() const {
  return actual_length_[kIdx];
}

BDM_SO_DEFINE(inline const std::array<double, 3>& NeuriteExt)::GetAxis() const {
  // local coordinate x_axis_ is equal to cylinder axis
  return x_axis_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::UpdateDependentPhysicalVariables() {
  spring_axis_[kIdx] = Matrix::Subtract(position_[kIdx], mother_[kIdx].GetPosition());
  actual_length_[kIdx] = std::sqrt(Matrix::Dot(spring_axis_[kIdx], spring_axis_[kIdx]));
  tension_[kIdx] = spring_constant_[kIdx] * (actual_length_[kIdx] - resting_length_[kIdx]) / resting_length_[kIdx];
  UpdateVolume();
  // TODO updateSpatialOrganizationNodePosition();
}

BDM_SO_DEFINE(inline void NeuriteExt)::InsertProximalCylinder(TMostDerived<Scalar>* new_neurite_element) {
  InsertProximalCylinder(new_neurite_element, 0.5);
}

BDM_SO_DEFINE(inline void NeuriteExt)::InsertProximalCylinder(TMostDerived<Scalar>* new_neurite_element, double distal_portion) {
  // location
  auto new_position = Matrix::Subtract(position_[kIdx], Matrix::ScalarMult(distal_portion, spring_axis_[kIdx]));

  // TODO
  // double temp = distal_portion + (1 - distal_portion) / 2.0;
  // std::array<double, 3> newProximalCylinderSpatialNodeLocation { mass_location_[0] - temp * spring_axis_[0],
  //     mass_location_[1] - temp * spring_axis_[1], mass_location_[2] - temp * spring_axis_[2] };

  new_neurite_element->position_[0] = new_position;

  // family relations
  mother_->UpdateRelative(this, new_neurite_element);
  new_neurite_element->SetMother(mother_[kIdx]);
  SetMother(new_neurite_element);
  new_neurite_element->SetDaughterLeft(this);
  // TODO SOM relation
  // auto new_son = so_node_->getNewInstance(newProximalCylinderSpatialNodeLocation, new_cylinder.get());  // todo catch PositionNotAllowedException
  // new_cylinder->setSoNode(std::move(new_son));
  // registering the new cylinder with ecm
  // TODO ecm_->addPhysicalCylinder(new_cylinder.get());
  // physics
  new_neurite_element->resting_length_[kIdx] = (1 - distal_portion) * resting_length_[kIdx];
  resting_length_[kIdx] *= distal_portion;

  //  TODO intracellularSubstances quantities .....................................
  // (concentrations are solved in updateDependentPhysicalVariables():
  // for (auto& pair : intracellular_substances_) {
  //   auto s = pair.second.get();
  //   // if doesn't diffuse at all : all the substance stays in the distal part !
  //   if (s->getDiffusionConstant() < 0.000000000001) {
  //     continue;
  //   }
  //   // create similar IntracellularSubstance and insert it into the new cylinder
  //   double quantity_before_distribution = s->getQuantity();
  //   auto s2 = IntracellularSubstance::UPtr(new IntracellularSubstance(*s));
  //   s2->setQuantity(quantity_before_distribution * (1 - distal_portion));
  //   new_cylinder->addNewIntracellularSubstance(std::move(s2));
  //   // decrease value of IntracellularSubstance in this cylinder
  //   s->setQuantity(quantity_before_distribution * distal_portion);
  // }
  UpdateDependentPhysicalVariables();
  new_neurite_element->UpdateDependentPhysicalVariables();
  // UpdateLocalCoordinateAxis has to come after UpdateDepend...
  new_neurite_element->UpdateLocalCoordinateAxis();

  // TODO // copy the LocalBiologicalModules (not done in NeuriteElement, because this creation of
  // // cylinder-neuriteElement is decided for physical and not biological reasons
  // for (auto module : neurite_element_->getLocalBiologyModulesList()) {
  //   if (module->isCopiedWhenNeuriteElongates())
  //     ne->addLocalBiologyModule(module->getCopy());
  // }

  // TODO deal with the excressences:
  // if (!excrescences_.empty()) {
  //   auto it = excrescences_.begin();
  //   do {
  //     auto ex_raw = (*it).get();
  //     auto pos = ex_raw->getPositionOnPO();
  //     // transmit them to proximal cyl
  //     if (pos[0] < new_cylinder->actual_length_) {
  //       ex_raw->setPo(new_cylinder.get());
  //       new_cylinder->addExcrescence(std::move(*it));
  //       excrescences_.erase(it);
  //       it--;
  //     } else {
  //       // or kep them here, depending on coordinate
  //       pos[0] -= new_cylinder->actual_length_;
  //       ex_raw->setPositionOnPO(pos);
  //     }
  //   } while (++it != excrescences_.end());
  // }
}

BDM_SO_DEFINE(inline void NeuriteExt)::ExtendSideCylinder(TMostDerived<Scalar>* new_branch, double length, const std::array<double, 3>& direction) {
  auto dir = direction;
  double angle_with_side_branch = Math::AngleRadian(spring_axis_[kIdx], direction);
  if (angle_with_side_branch < 0.78 || angle_with_side_branch > 2.35) {  // 45-135 degrees
    auto p = Math::CrossProduct(spring_axis_[kIdx], direction);
    p = Math::CrossProduct(p, spring_axis_[kIdx]);
    dir = Matrix::Add(Math::Normalize(direction), Math::Normalize(p));
  }
  // location of mass and computation center
  auto new_spring_axis = Matrix::ScalarMult(length, Math::Normalize(direction));
  new_branch->position_[0] = Matrix::Add(position_[kIdx], new_spring_axis);
  new_branch->spring_axis_[0] = new_spring_axis;
  // physics
  new_branch->actual_length_[0] = length;
  new_branch->SetRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
  new_branch->SetDiameter(Param::kNeuriteDefaultDiameter, true);
  new_branch->UpdateLocalCoordinateAxis();
  // family relations
  new_branch->SetMother(this);
  daughter_right_[kIdx] = new_branch;
  // TODO new CentralNode
  // auto new_center_location = Matrix::add(mass_location_, Matrix::scalarMult(0.5, new_spring_axis));
  // auto new_son = so_node_->getNewInstance(new_center_location, new_branch.get());  // todo catch PositionNotAllowedException
  // new_branch->setSoNode(std::move(new_son));

  // correct physical values (has to be after family relations and SON assignement).
  new_branch->UpdateDependentPhysicalVariables();
  // register to ecm
  // TODO ecm_->addPhysicalCylinder(new_branch.get());

  // i'm scheduled to run physics next time :
  // (the side branch automatically is too, because it's a new PhysicalObject)
  // TODO setOnTheSchedulerListForPhysicalObjects(true);
}

BDM_SO_DEFINE(inline void NeuriteExt)::RemoveProximalCylinder() {
  // The mother is removed if (a) it is a PhysicalCylinder and (b) it has no other daughter than
  if (!mother_[kIdx].IsNeurite() || !mother_[kIdx].GetDaughterRight().IsNullPtr()) {
    return;
  }
  // the ex-mother's neurite Element has to be removed
  // TODO proximal_cylinder->getNeuriteElement()->removeYourself();
  // Re-organisation of the PhysicalObject tree structure: by-passing proximalCylinder
  mother_[kIdx].GetMother()->UpdateRelative(mother_[kIdx], this);
  SetMother(mother_[kIdx].GetMother());

  // TODO collecting (the quantities of) the intracellular substances of the removed cylinder.
  // for (auto s : proximal_cylinder->getIntracellularSubstances1()) {
  //   modifyIntracellularQuantity(s->getId(), s->getQuantity() / Param::simulation_time_step_);
  //   // divided by time step, because in the method the parameter is multiplied by time step...
  //   // and we want to change the quantity.
  //   // We don't change the concentration, it is done later by the call to updateVolume()
  // }

  // Keeping the same tension :
  // (we don't use updateDependentPhysicalVariables(), because we have tension and want to
  // compute restingLength, and not the opposite...)
  // T = k*(A-R)/R --> R = k*A/(T+K)
  spring_axis_[kIdx] = Matrix::Subtract(position_[kIdx], mother_[kIdx].GetPosition());
  actual_length_[kIdx] = Math::Norm(spring_axis_[kIdx]);
  resting_length_[kIdx] = spring_constant_[kIdx] * actual_length_[kIdx] / (tension_[kIdx] + spring_constant_[kIdx]);
  // .... and volume_
  UpdateVolume();
  // and local coord
  UpdateLocalCoordinateAxis();
  // ecm
  // TODO ecm_->removePhysicalCylinder(proximal_cylinder);

  // dealing with excressences:
  // mine are shifted up :
  // double shift = actual_length_ - proximal_cylinder->actual_length_;
  // for (auto& ex : excrescences_) {
  //   auto pos = ex->getPositionOnPO();
  //   pos[0] += shift;
  //   ex->setPositionOnPO(pos);
  // }
  // // I incorporate the ones of the previous cyl:
  // for (auto& ex : proximal_cylinder->excrescences_) {
  //   excrescences_.push_back(std::move(ex));
  //   ex->setPo(this);
  // }
  // // TODO: take care of Physical Bonds
  // proximal_cylinder->setStillExisting(false);
  // the SON
  // TODO updateSpatialOrganizationNodePosition();
  // TODO: CAUTION : for future parallel implementation. If a visitor is in the branch, it gets destroyed....
}

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURITE_H_
