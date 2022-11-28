// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef NEUROSCIENCE_NEURITE_ELEMENT_H_
#define NEUROSCIENCE_NEURITE_ELEMENT_H_

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/agent/agent.h"
#include "core/interaction_force.h"
#include "core/scheduler.h"
#include "core/shape.h"
#include "core/util/log.h"
#include "core/util/math.h"
#include "core/util/random.h"
#include "neuroscience/neuron_or_neurite.h"
#include "neuroscience/neuron_soma.h"
#include "neuroscience/new_agent_event/neurite_bifurcation_event.h"
#include "neuroscience/new_agent_event/neurite_branching_event.h"
#include "neuroscience/new_agent_event/new_neurite_extension_event.h"
#include "neuroscience/new_agent_event/side_neurite_extension_event.h"
#include "neuroscience/new_agent_event/split_neurite_element_event.h"
#include "neuroscience/param.h"

namespace bdm {
namespace neuroscience {

/// Class defining a neurite element with cylindrical geometry.
/// A cylinder can be seen as a normal cylinder, with two end points and a
/// diameter. It is oriented; the two points are called proximal and distal.
/// The neurite element is be part of a tree-like structure with (one and only)
/// one object at its proximal point and (up to) two neurite elements at
/// its distal end. The proximal end can be a Neurite or Neuron cell body.
/// If there is only one daughter, it is the left one.
/// If `daughter_left_ == nullptr`, there is no distal neurite element.
/// (it is a terminal neurite element). The presence of a `daughter_left_`
/// means that this branch has a bifurcation at its distal end.
/// \n
/// All the mass of the neurite element is concentrated at the distal point.
/// Only the distal end is moved. All the forces that are applied to the
/// proximal node are transmitted to the mother element
class NeuriteElement : public Agent, public NeuronOrNeurite {
  BDM_AGENT_HEADER(NeuriteElement, Agent, 1);

 public:
  NeuriteElement();

  void Initialize(const NewAgentEvent& event) override;

  void Update(const NewAgentEvent& event) override;

  const AgentUid& GetUid() const override { return Base::GetUid(); }

  Spinlock* GetLock() override { return Base::GetLock(); }

  void CriticalRegion(std::vector<AgentPointer<>>* aptrs) override;

  Shape GetShape() const override { return Shape::kCylinder; }

  /// Returns the data members that are required to visualize this simulation
  /// object.
  std::set<std::string> GetRequiredVisDataMembers() const override;

  void SetDiameter(real_t diameter) override;

  void SetDensity(real_t density);

  const Real3& GetPosition() const override { return position_; }

  void SetPosition(const Real3& position) override;

  void UpdatePosition();

  /// return end of neurite element position
  const Real3& GetMassLocation() const { return mass_location_; }

  void SetMassLocation(const Real3& mass_location);

  real_t GetAdherence() const { return adherence_; }

  void SetAdherence(real_t adherence);

  const Real3& GetXAxis() const { return x_axis_; }
  const Real3& GetYAxis() const { return y_axis_; }
  const Real3& GetZAxis() const { return z_axis_; }

  real_t GetVolume() const { return volume_; }

  real_t GetDiameter() const override { return diameter_; }

  real_t GetDensity() const { return density_; }

  real_t GetMass() const { return density_ * volume_; }

  /// Returns the absolute coordinates of the location where the daughter is
  /// attached.
  /// @param daughter_element_idx element_idx of the daughter
  /// @return the coord
  Real3 OriginOf(const AgentUid& daughter_uid) const override;

  /// Returns StructureIdentifierSWC:kAxon if NeuriteElement is an Axon and
  /// StructureIdentifierSWC::kApicalDendrite for all other cases.
  virtual StructureIdentifierSWC GetIdentifierSWC() const override;

  // TODO(neurites) arrange in order end

  /// Retracts the neurite element, if it is a terminal one.
  /// Branch retraction by moving the distal end toward the
  /// proximal end (the mother), maintaining the same tension in the
  /// neurite element. The method shortens the actual and the resting length
  /// so that the result is a shorter neurite element with the same tension.
  ///   * If this neurite element is longer than the required shortening, it
  ///   simply retracts.
  ///   * If it is shorter and its mother has no other daughter, it merges with
  ///   it's mother and the method is recursively called (this time the cylinder
  ///   length is bigger because we have a new neurite element that resulted
  ///   from the fusion of two).
  ///   * If it is shorter and either the previous neurite element has another
  ///   daughter or the mother is not a neurite element, it disappears.
  /// @param speed the retraction speed in microns / h
  void RetractTerminalEnd(real_t speed);

  /// Method used for active extension of a terminal branch, representing the
  /// steering of a growth cone. The movement should always be forward,
  /// otherwise no movement is performed.
  /// If `direction` points in an opposite direction than the axis, i.e.
  /// if the dot product is negative, there is no movement (only elongation is
  /// possible).
  /// @param speed
  /// @param direction
  void ElongateTerminalEnd(real_t speed, const Real3& direction);

  /// Returns true if a side branch is physically possible. That is if this is
  /// not a terminal  branch and if there is not already a second daughter.
  bool BranchPermitted() const;

  /// \brief Create a branch for this neurite element.
  ///
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch(real_t new_branch_diameter, const Real3& direction,
                         real_t length = 1.0);

  /// \brief Create a branch for this neurite element.
  ///
  /// Diameter of new side branch will be equal to this neurites diameter.
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch(const Real3& direction);

  /// \brief Create a branch for this neurite element.
  ///
  /// Use a random growth direction for the side branch.
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch(real_t diameter);

  /// \brief Create a branch for this neurite element.
  ///
  /// Use a random growth direction for the side branch.
  /// Diameter of new side branch will be equal to this neurites diameter.
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch();

  /// Returns true if a bifurcation is physicaly possible. That is if the
  /// neurite element has no daughter and the actual length is bigger than the
  /// minimum required.
  bool BifurcationPermitted() const;
  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate(real_t length, real_t diameter_1,
                                           real_t diameter_2,
                                           const Real3& direction_1,
                                           const Real3& direction_2);

  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate(real_t diameter_1, real_t diameter_2,
                                           const Real3& direction_1,
                                           const Real3& direction_2);

  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate(const Real3& direction_1,
                                           const Real3& direction_2);

  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate();

  // ***************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE *
  // ***************************************************************************

  // TODO(neurites) documentation
  void RemoveDaughter(const AgentPointer<NeuriteElement>& daughter) override;

  // TODO(neurites) add documentation
  void UpdateRelative(const NeuronOrNeurite& old_relative,
                      const NeuronOrNeurite& new_relative) override;

  /// Returns the total force that this `NeuriteElement` exerts on it's mother.
  /// It is the sum of the spring force and the part of the inter-object force
  /// computed earlier in `CalculateDisplacement`
  Real3 ForceTransmittedFromDaugtherToMother(const NeuronOrNeurite& mother);

  // ***************************************************************************
  //   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
  // ***************************************************************************

  /// Checks if this NeuriteElement is either too long or too short.
  ///   * too long: insert another NeuriteElement
  ///   * too short fuse it with the proximal element or even delete it
  ///
  /// Only executed for terminal neurite elements.
  void RunDiscretization() override;

  // ***************************************************************************
  //   ELONGATION, RETRACTION, BRANCHING
  // ***************************************************************************

  /// Method used for active extension of a terminal branch, representing the
  /// steering of a
  /// growth cone. There is no check for real extension (unlike in
  /// `ExtendCylinder()`` ).
  ///
  /// @param speed      of the growth rate (microns/hours).
  /// @param direction  the 3D direction of movement.
  void MovePointMass(real_t speed, const Real3& direction);

  void SetRestingLengthForDesiredTension(real_t tension);
  /// Progressive modification of the volume. Updates the diameter.
  /// @param speed cubic micron/ h
  void ChangeVolume(real_t speed);
  /// Progressive modification of the diameter. Updates the volume.
  /// @param speed micron/ h
  void ChangeDiameter(real_t speed);

  struct MechanicalForcesFunctor : public Functor<void, Agent*, real_t> {
    const InteractionForce* force;
    NeuriteElement* ne;
    Real3& force_from_neighbors;
    Real3& force_on_my_mothers_point_mass;
    real_t& h_over_m;
    bool& has_neurite_neighbor;
    uint64_t& non_zero_neighbor_force;

    MechanicalForcesFunctor(const InteractionForce* force,
                            NeuriteElement* neurite,
                            Real3& force_from_neighbors,
                            Real3& force_on_my_mothers_point_mass,
                            real_t& h_over_m, bool& has_neurite_neighbor,
                            uint64_t& non_zero_neighbor_force)
        : force(force),
          ne(neurite),
          force_from_neighbors(force_from_neighbors),
          force_on_my_mothers_point_mass(force_on_my_mothers_point_mass),
          h_over_m(h_over_m),
          has_neurite_neighbor(has_neurite_neighbor),
          non_zero_neighbor_force(non_zero_neighbor_force) {}

    void operator()(Agent* neighbor, real_t squared_distance) override;
  };

  // ***************************************************************************
  //   Physics
  // ***************************************************************************

  Real3 CalculateDisplacement(const InteractionForce* force,
                              real_t squared_radius, real_t dt) override;

  // TODO(neurites) documentation
  void ApplyDisplacement(const Real3& displacement) override;

  /// Defines the three orthonormal local axis so that a cylindrical coordinate
  /// system can be used. The `x_axis_` is aligned with the `spring_axis_`.
  /// The two other are in the plane perpendicular to `spring_axis_`.
  /// This method to update the axis was suggested by Matt Coock.
  /// Although not perfectly exact, it is accurate enough for us to use.
  void UpdateLocalCoordinateAxis();
  /// Recomputes diameter after volume has changed.
  void UpdateDiameter();
  /// Recomputes volume, after diameter has been changed.
  void UpdateVolume();

  // ***************************************************************************
  //   Coordinates transform
  // ***************************************************************************

  /// 3 systems of coordinates :
  ///
  /// Global :   cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0)
  /// and (0,0,1)
  ///        with origin at (0,0,0).
  /// Local :    defined by orthogonal axis xAxis (=vect proximal to distal
  /// end), yAxis and zAxis,
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
  /// of a point expressed in global cartesian coordinates
  /// ([1,0,0],[0,1,0],[0,0,1]).
  /// @param position in global coordinates
  Real3 TransformCoordinatesGlobalToLocal(const Real3& position) const;

  /// L -> G
  /// Returns the position in global cartesian coordinates
  /// ([1,0,0],[0,1,0],[0,0,1])
  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
  /// @param position in local coordinates
  Real3 TransformCoordinatesLocalToGlobal(const Real3& position) const;

  ///  L -> P
  /// Returns the position in cylindrical coordinates (h,theta,r)
  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
  /// @param position in local coordinates
  Real3 TransformCoordinatesLocalToPolar(const Real3& position) const;

  /// P -> L
  /// Returns the position in the local coordinate system (xAxis, yXis, zAxis)
  /// of a point expressed in cylindrical coordinates (h,theta,r).
  /// @param position in local coordinates
  Real3 TransformCoordinatesPolarToLocal(const Real3& position) const;

  /// P -> G :    P -> L, then L -> G
  Real3 TransformCoordinatesPolarToGlobal(
      const std::array<real_t, 2>& position) const;

  /// G -> L :    G -> L, then L -> P
  Real3 TransformCoordinatesGlobalToPolar(const Real3& position) const;

  // ***************************************************************************
  //   GETTERS & SETTERS
  // ***************************************************************************

  bool IsAxon() const { return is_axon_; }

  void SetAxon(bool is_axon) { is_axon_ = is_axon; }

  AgentPointer<NeuronOrNeurite>& GetMother() { return mother_; }

  const AgentPointer<NeuronOrNeurite>& GetMother() const { return mother_; }

  void SetMother(const AgentPointer<NeuronOrNeurite>& mother) {
    mother_ = mother;
  }

  /// @return the (first) distal neurite element, if it exists,
  /// i.e. if this is not the terminal segment (otherwise returns nullptr).
  const AgentPointer<NeuriteElement>& GetDaughterLeft() const;

  void SetDaughterLeft(const AgentPointer<NeuriteElement>& daughter);

  /// @return the second distal neurite element, if it exists
  /// i.e. if there is a branching point just after this element (otherwise
  /// returns nullptr).
  const AgentPointer<NeuriteElement>& GetDaughterRight() const;

  void SetDaughterRight(const AgentPointer<NeuriteElement>& daughter);

  int GetBranchOrder() const { return branch_order_; }

  void SetBranchOrder(int branch_order) { branch_order_ = branch_order; }

  real_t GetActualLength() const { return actual_length_; }

  /// Should not be used, since the actual length depends on the geometry.
  void SetActualLength(real_t actual_length);

  real_t GetRestingLength() const { return resting_length_; }

  void SetRestingLength(real_t resting_length);

  const Real3& GetSpringAxis() const { return spring_axis_; }

  void SetSpringAxis(const Real3& axis);

  real_t GetSpringConstant() const { return spring_constant_; }

  void SetSpringConstant(real_t spring_constant);

  real_t GetTension() const { return tension_; }

  void SetTension(real_t tension);

  /// NOT A "REAL" GETTER
  /// Gets a vector of length 1, with the same direction as the SpringAxis.
  /// @return a normalized spring axis
  Real3 GetUnitaryAxisDirectionVector() const;

  /// Should return yes if the PhysicalCylinder is considered a terminal branch.
  /// @return is it a terminal branch
  bool IsTerminal() const { return daughter_left_ == nullptr; }

  /// retuns the position of the proximal end, ie the position minus the spring
  /// axis.
  /// Is mainly used for paint
  Real3 ProximalEnd() const { return mass_location_ - spring_axis_; }

  /// Returns the position of the distal end == position_
  const Real3& DistalEnd() const { return mass_location_; }

  /// Returns the total (actual) length of all the neurite elements (including
  /// the one in which this method is
  /// called) before the previous branching point. Used to decide if long enough
  /// to bifurcate or branch,
  /// independently of the discretization.
  real_t LengthToProximalBranchingPoint() const;

  real_t GetLength() const { return actual_length_; }

  /// Returns the axis direction of a neurite element
  const Real3& GetAxis() const;

  /// Updates the spring axis, the actual length, the tension and the volume.
  ///
  /// For tension, `T = k * (aL - rL) / rL`.  k = spring constant,
  /// rL = resting length, aL = actual length. (Note the division by rL.
  /// Otherwise we could have cylinders with big aL and rL = 0).\n
  void UpdateDependentPhysicalVariables() override;

  friend std::ostream& operator<<(std::ostream& str, const NeuriteElement& n);

 protected:
  void Copy(const NeuriteElement& rhs);

 private:
  // TODO(lukas) data members same as in cell -> resolve once ROOT-9321 has been
  // resolved
  /// mass_location_ is distal end of the cylinder
  /// NB: Use setter and don't assign values directly
  Real3 mass_location_ = {{0.0, 0.0, 0.0}};

  /// position_ is the middle point of cylinder
  Real3 position_ = {{0.0, 0.0, 0.0}};

  real_t volume_;
  /// NB: Use setter and don't assign values directly
  real_t diameter_ = 1;
  /// NB: Use setter and don't assign values directly
  real_t density_;
  /// NB: Use setter and don't assign values directly
  real_t adherence_;
  /// First axis of the local coordinate system equal to cylinder axis
  Real3 x_axis_ = {{1.0, 0.0, 0.0}};
  /// Second axis of the local coordinate system.
  Real3 y_axis_ = {{0.0, 1.0, 0.0}};
  /// Third axis of the local coordinate system.
  Real3 z_axis_ = {{0.0, 0.0, 1.0}};

  bool is_axon_ = false;

  /// Parent node in the neuron tree structure can be a Neurite element
  /// or cell body
  AgentPointer<NeuronOrNeurite> mother_;

  /// First child node in the neuron tree structure (can only be a Neurite
  /// element)
  AgentPointer<NeuriteElement> daughter_left_;
  /// Second child node in the neuron tree structure. (can only be a Neurite
  /// element)
  AgentPointer<NeuriteElement> daughter_right_;

  /// number of branching points from here to the soma (root of the neuron
  /// tree-structure).
  int branch_order_ = 0;

  /// The part of the inter-object force transmitted to the mother (parent node)
  Real3 force_to_transmit_to_proximal_mass_ = {{0, 0, 0}};

  /// from the attachment point to the mass location
  /// (proximal -> distal).
  /// NB: Use setter and don't assign values directly
  Real3 spring_axis_ = {{0, 0, 0}};

  /// Real length of the PhysicalCylinder (norm of the springAxis).
  /// NB: Use setter and don't assign values directly
  real_t actual_length_ = 1;

  /// Tension in the cylinder spring.
  /// NB: Use setter and don't assign values directly
  real_t tension_;

  /// Spring constant per distance unit (springConstant restingLength  = "real"
  /// spring constant).
  real_t spring_constant_;

  /// The length of the internal spring where tension would be zero.
  /// T = k*(A-R)/R --> R = k*A/(T+K)
  real_t resting_length_;

  /// Helper variable needed in CalculateDisplacement
  bool has_neurite_neighbor_ = false;

  /// \brief Split this neurite element into two segments.
  ///
  /// \see SplitNeuriteElementEvent
  NeuriteElement* SplitNeuriteElement(real_t distal_portion = 0.5);

  /// Merges two neurite elements together. The one in which the method is
  /// called phagocytes it's mother.
  void RemoveProximalNeuriteElement();

  /// \brief Extend a side neurite element and assign it to daughter right.
  ///
  /// \see SideNeuriteExtensionEvent
  NeuriteElement* ExtendSideNeuriteElement(real_t length, real_t diameter,
                                           const Real3& direction);

  /// TODO
  void InitializeNewNeuriteExtension(NeuronSoma* soma, real_t diameter,
                                     real_t phi, real_t theta);

  /// TODO
  void InitializeNeuriteBifurcation(NeuriteElement* mother, real_t length,
                                    real_t diameter, const Real3& direction);

  /// Neurite branching is composed of neurite splitting and side neurite
  /// extension. To avoid code duplication in constructors, logic has been moved
  /// here.
  /// \see SplitNeuriteElementEvent, NeuriteBranchingEvent
  void InitializeSplitOrBranching(NeuriteElement* other, real_t distal_portion);

  /// Neurite branching is composed of neurite splitting and side neurite
  /// extension. To avoid code duplication in constructors, logic has been moved
  /// here.
  void InitializeSideExtensionOrBranching(NeuriteElement* mother, real_t length,
                                          real_t diameter,
                                          const Real3& direction);
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURITE_ELEMENT_H_
