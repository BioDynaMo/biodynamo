#ifndef NEUROSCIENCE_NEURITE_H_
#define NEUROSCIENCE_NEURITE_H_

#include "biology_module_util.h"
#include "default_force.h"
#include "shape.h"
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

// Declare biology module events for neurites
extern const BmEvent gNeuriteElongation;
extern const BmEvent gNeuriteBranching;
extern const BmEvent gNeuriteBifurcation;
extern const BmEvent gNeuriteSideCylinderExtension;


/// The mother of a neurite can either be a neuron or a neurite.
/// Therefore, this class acts as an intermediate layer that forwards function
/// calls to the correct object.
/// @tparam TNeuronSoPtr   type of Neuron simulation object pointer
/// @tparam TNeuriteSoPtr  type of Neurite simulation object pointer.
template<typename TNeuronSoPtr, typename TNeuriteSoPtr>
class NeuronNeuriteAdapter {
public:
  using Self = NeuronNeuriteAdapter<TNeuronSoPtr, TNeuriteSoPtr>;

  NeuronNeuriteAdapter() {}

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

  const TNeuronSoPtr& GetNeuronSoPtr() const {
    return neuron_ptr_;
  }

  const TNeuriteSoPtr& GetNeuriteSoPtr() const {
    return neurite_ptr_;
  }

  TNeuronSoPtr& GetNeuronSoPtr() { return neuron_ptr_; }

  TNeuriteSoPtr& GetNeuriteSoPtr() { return neurite_ptr_; }

  const std::array<double, 3> GetPosition() const {
    if (IsNeurite()) {
      return neurite_ptr_.Get().GetPosition();
    }
    assert(IsNeuron() && "Initialization error: neither neuron nor neurite");
    return neuron_ptr_.Get().GetPosition();
  }

  std::array<double, 3> OriginOf(uint32_t daughter_element_idx) const {
    if (IsNeurite()) {
      return neurite_ptr_.Get().OriginOf(daughter_element_idx);
    }
    assert(IsNeuron() && "Initialization error: neither neuron nor neurite");
    return neuron_ptr_.Get().OriginOf(daughter_element_idx);
  }

  bool IsNeuron() const { return !neuron_ptr_.IsNullPtr(); }
  bool IsNeurite() const { return !neurite_ptr_.IsNullPtr(); }

  // TODO LB reference?
  Self GetMother() {
    assert(IsNeurite() && "This function call is only allowed for a Neurite");
    return neurite_ptr_.Get().GetMother();
  }

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
    if (IsNeurite()) {
      neurite_ptr_.Get().UpdateDependentPhysicalVariables();
      return;
    }
    assert(IsNeuron() && "Initialization error: neither neuron nor neurite");
    neuron_ptr_.Get().UpdateVolume();
  }

  template <typename TNeuronOrNeurite>
  void UpdateRelative(const TNeuronOrNeurite& old_rel, const TNeuronOrNeurite& new_rel) {
    if (IsNeurite()) {
      neurite_ptr_.Get().UpdateRelative(old_rel, new_rel);
      return;
    }
    // TODO improve
    auto old_neurite_soptr = old_rel.GetNeuriteSoPtr();
    auto new_neurite_soptr = new_rel.GetNeuriteSoPtr();
    assert(IsNeuron() && "Initialization error: neither neuron nor neurite");
    neuron_ptr_.Get().UpdateRelative(old_neurite_soptr, new_neurite_soptr);
  }

  auto RemoveYourself() -> decltype(std::declval<TNeuriteSoPtr>().Get().RemoveYourself()) const {
    assert(IsNeurite() && "This function call is only allowed for a Neurite");
    return neurite_ptr_.Get().RemoveYourself();
  }

  void RemoveDaughter(const TNeuriteSoPtr& mother) {
    if(IsNeurite()) {
      neurite_ptr_.Get().RemoveDaughter(mother);
      return;
    }
    assert(IsNeuron() && "Initialization error: neither neuron nor neurite");
    neuron_ptr_.Get().RemoveDaughter(mother);
  }

  bool operator==(const NeuronNeuriteAdapter<TNeuronSoPtr, TNeuriteSoPtr> other ) const {
    return neuron_ptr_ == other.neuron_ptr_ && neurite_ptr_ == other.neurite_ptr_;
  }

  bool operator!=(const NeuronNeuriteAdapter<TNeuronSoPtr, TNeuriteSoPtr> other ) const {
    return !(*this == other);
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
BDM_SIM_OBJECT(Neurite, bdm::SimulationObject) {
  BDM_SIM_OBJECT_HEADER(NeuriteExt, 1, biology_modules_, mass_location_, volume_, diameter_, adherence_, x_axis_, y_axis_, z_axis_, box_idx_,
                                  is_axon_,
                                  mother_, daughter_left_, daughter_right_, branch_order_, force_to_transmit_to_proximal_mass_, spring_axis_, actual_length_, tension_, spring_constant_, resting_length_);

  using TNeuron = typename TCompileTimeParam::TNeuron;

  // using NeuriteOrNeuron = NeuronNeuriteAdapter<Convert<TNeuron, SimBackend>, MostDerivedSB, SimBackend>;
  using NeuriteOrNeuron = NeuronNeuriteAdapter<ToSoPtr<TNeuron>, MostDerivedSoPtr>;

  // SoPointer<typename ToBackend<TNeuron, Scalar>::type, SimBackend> aa_;
  ToSoPtr<TNeuron> aa_;

 public:
   /// Returns the data members that are required to visualize this simulation
   /// object.
   static std::set<std::string> GetRequiredVisDataMembers() {
     return {"mass_location_", "diameter_", "actual_length_", "spring_axis_"};
   }

   static constexpr Shape GetShape() { return kCylinder; }

   NeuriteExt() {}

  /// Update references of simulation objects that changed its memory position.
  /// @param update_info vector index = type_id, map stores (old_index -> new_index)
  void UpdateReferences(const std::vector<std::unordered_map<uint32_t, uint32_t>>& update_info) {
   using TRm = std::remove_pointer_t<decltype(Rm())>;

   constexpr int neurite_type_idx = TRm::template GetTypeIndex<MostDerived>();
   const auto& neurite_updates = update_info[neurite_type_idx];

    this->UpdateReference(&daughter_right_[kIdx], neurite_updates);
    this->UpdateReference(&daughter_left_[kIdx], neurite_updates);
    if (mother_[kIdx].IsNeurite()) {
      this->UpdateReference(&(mother_[kIdx].GetNeuriteSoPtr()), neurite_updates);
    } else if (mother_[kIdx].IsNeuron()) {
      constexpr int neuron_type_idx = TRm::template GetTypeIndex<TNeuron>();
      const auto& neuron_updates = update_info[neuron_type_idx];
      this->UpdateReference(&(mother_[kIdx].GetNeuronSoPtr()), neuron_updates);
    }
  }

   void SetDiameter(double diameter) {
     diameter_[kIdx] = diameter;
     UpdateVolume();
   }
   const std::array<double, 3> GetPosition() const { return Matrix::Subtract(mass_location_[kIdx], Matrix::ScalarMult(0.5, spring_axis_[kIdx])); }
   void SetPosition(const std::array<double, 3>& position) { mass_location_[kIdx] = Matrix::Add(position, Matrix::ScalarMult(0.5, spring_axis_[kIdx])); }
   // TODO reevaluate if method is required or data member should be used
   /// return end of cylinder position
   const std::array<double, 3>& GetMassLocation() const {
     return mass_location_[kIdx];
   }
   // TODO reevaluate if method is required or data member should be used
   void SetMassLocation(const std::array<double, 3>& mass_location) {
     mass_location_[kIdx] = mass_location;
   }

   double GetAdherence() const { return adherence_[kIdx]; }
   void SetAdherence(double adherence) { adherence_[kIdx] = adherence; }

   const std::array<double, 3>& GetXAxis() const { return x_axis_[kIdx]; }
   const std::array<double, 3>& GetYAxis() const { return y_axis_[kIdx]; }
   const std::array<double, 3>& GetZAxis() const { return z_axis_[kIdx]; }

   double GetVolume() const { return volume_[kIdx]; }
   double GetDiameter() const { return diameter_[kIdx]; }

   uint64_t GetBoxIdx() const { return box_idx_[kIdx]; }
   void SetBoxIdx(uint64_t idx) { box_idx_[kIdx] = idx; }

   /// Returns the absolute coordinates of the location where the dauther is
   /// attached.
   /// @param daughter_element_idx element_idx of the daughter
   /// @return the coord
   std::array<double, 3> OriginOf(uint32_t daughter_element_idx) const {
     return mass_location_[kIdx];
   }

   using TBiologyModuleVariant = typename TCompileTimeParam::BiologyModules;

   /// Add a biology module to this cell
   /// @tparam TBiologyModule type of the biology module. Must be in the set of
   ///         types specified in `TBiologyModuleVariant`
   template <typename TBiologyModule>
   void AddBiologyModule(TBiologyModule && module) {
     biology_modules_[kIdx].emplace_back(module);
   }

   /// Execute all biology modules
   void RunBiologyModules() {
     RunVisitor<TMostDerived<Backend>> visitor(static_cast<TMostDerived<Backend>*>(this));
     for (auto& module : biology_modules_[kIdx]) {
       visit(visitor, module);
     }
   }

   void SetBiologyModules(std::vector<TBiologyModuleVariant>&& bms) {
     biology_modules_[kIdx] = bms;
   }

   // TODO arrange in order end

   // TODO should be generated
   double* GetPositionPtr() { return &(mass_location_[0][0]); }  // FIXME
   double* GetDiameterPtr() { return &(diameter_[0]); }
   /// TODO generated end`

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
  MostDerivedSoPtr Branch(double new_branch_diameter, const std::array<double, 3>& direction);

  /// Makes a side branch, i.e. splits this neurite element into two and puts a daughter right at the proximal half.
  /// @param direction growth direction, but will be automatically corrected if not at least 45 degrees from the cylinder's axis.
  MostDerivedSoPtr Branch(const std::array<double, 3>& direction);

  /// Makes a side branch, i.e. splits this neurite element into two and puts a daughter right at the proximal half.
  /// @param diameter of the side branch
  MostDerivedSoPtr Branch(double diameter);

  /// Makes a side branch, i.e. splits this neurite element into two and puts a daughter right at the proximal half.
  MostDerivedSoPtr Branch();

  /// Returns true if a bifurcation is physicaly possible. That is if the neurite element
  /// has no daughter and the actual length is bigger than the minimum required.
  bool BifurcationPermitted() const;

  /// Bifurcation of a growth come (only works for terminal segments).
  /// Note : angles are corrected if they are pointing backward.
  /// @param diameter_1  of new daughterLeft
  /// @param diameter_2 of new daughterRight
  /// @param direction_1
  /// @param direction_2
  /// \return
  std::array<MostDerivedSoPtr, 2> Bifurcate(double diameter_1, double diameter_2,
                                           const std::array<double, 3>& direction_1,
                                           const std::array<double, 3>& direction_2);

  /// Bifurcation of a growth come (only works for terminal segments).
  /// Note : angles are corrected if they are pointing backward.
  /// @param length of new branches
  /// @param diameter_1  of new daughterLeft
  /// @param diameter_2 of new daughterRight
  /// @param direction_1
  /// @param direction_2
  /// \return
  std::array<MostDerivedSoPtr, 2> Bifurcate(double length, double diameter_1, double diameter_2,
                                           const std::array<double, 3>& direction_1,
                                           const std::array<double, 3>& direction_2);

  std::array<MostDerivedSoPtr, 2> Bifurcate(const std::array<double, 3>& direction_1,
                                           const std::array<double, 3>& direction_2);

  std::array<MostDerivedSoPtr, 2> Bifurcate();


  // *************************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE                                            *
  // *************************************************************************************

  void RemoveDaughter(const MostDerivedSoPtr& daughter);

  // TODO add documentation
  void UpdateRelative(const NeuriteOrNeuron& old_relative,
                      const NeuriteOrNeuron& new_relative);

  /// Returns the total force that this `Neurite` exerts on it's mother.
  /// It is the sum of the spring force and the part of the inter-object force
  /// computed earlier in `RunPhyiscs`
  // TODO update Documentation (CalculateDisplacement is probably displacement op)
  std::array<double, 3> ForceTransmittedFromDaugtherToMother(const NeuriteOrNeuron& mother);

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

  void SetRestingLengthForDesiredTension(double tension);

  /// Progressive modification of the volume. Updates the diameter.
  /// @param speed cubic micron/ h
  void ChangeVolume(double speed);

  /// Progressive modification of the diameter. Updates the volume.
  /// @param speed micron/ h
  void ChangeDiameter(double speed);
 //
 //  // *************************************************************************************
 //  //   Physics
 //  // *************************************************************************************
 //
  // TODO documentatio, rename (maybe mechanical interactions)
  template <typename TGrid>
  std::array<double, 3> CalculateDisplacement(TGrid* grid, double squared_radius);

  void ApplyDisplacement(const std::array<double, 3>& displacement);

  /// Defines the three orthonormal local axis so that a cylindrical coordinate system
  /// can be used. The xAxis is aligned with the `spring_axis_`. The two other are in the
  /// plane perpendicular to `spring_axis_`. This method to update the axis was suggested by
  /// Matt Coock. - Although not perfectly exact, it is accurate enough for us to use.
  void UpdateLocalCoordinateAxis();

  /// Recomputes diameter after volume has changed.*/
  void UpdateDiameter();

  /// Recomputes volume, after diameter has been change.
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

  // FIXME
  // const NeuriteOrNeuron& GetMother() const;
  NeuriteOrNeuron GetMother();

  void SetMother(const NeuriteOrNeuron& mother);

  /// @return the (first) distal neurite element, if it exists,
  /// i.e. if this is not the terminal segment (otherwise returns nullptr).
  const MostDerivedSoPtr& GetDaughterLeft() const;

  void SetDaughterLeft(const MostDerivedSoPtr& daughter);

  /// @return the second distal neurite element, if it exists
  /// i.e. if there is a branching point just after this element (otherwise returns nullptr).
  // TODO rename GetRightDaughter
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

  void RemoveYourself();

  // TODO
  friend std::ostream& operator<<(std::ostream& str, const Self<Backend>& n) {
    auto pos = n.GetPosition();
    str << "MassLocation:     " << n.mass_location_[n.kIdx][0] << ", " << n.mass_location_[n.kIdx][1] << ", " << n.mass_location_[n.kIdx][2] << ", " << std::endl;
    str << "Position:         " << pos[0] << ", " << pos[1] << ", " << pos[2] << ", " << std::endl;
    str << "x_axis_:          " << n.x_axis_[n.kIdx][0] << ", " << n.x_axis_[n.kIdx][1] << ", " << n.x_axis_[n.kIdx][2] << ", " << std::endl;
    str << "y_axis_:          " << n.y_axis_[n.kIdx][0] << ", " << n.y_axis_[n.kIdx][1] << ", " << n.y_axis_[n.kIdx][2] << ", " << std::endl;
    str << "z_axis_:          " << n.z_axis_[n.kIdx][0] << ", " << n.z_axis_[n.kIdx][1] << ", " << n.z_axis_[n.kIdx][2] << ", " << std::endl;
    // str << "ftttpm:  " << n.force_to_transmit_to_proximal_mass_[n.kIdx][0] << ", " << n.force_to_transmit_to_proximal_mass_[n.kIdx][1] << ", " << n.force_to_transmit_to_proximal_mass_[n.kIdx][2] << ", " << std::endl;
    str << "spring_axis_:     " << n.spring_axis_[n.kIdx][0] << ", " << n.spring_axis_[n.kIdx][1] << ", " << n.spring_axis_[n.kIdx][2] << ", " << std::endl;
    str << "volume_:          " << n.volume_[n.kIdx]  << std::endl;
    str << "diameter_:        " << n.diameter_[n.kIdx]  << std::endl;
    // str << "is_axon_:  " << n.is_axon_[n.kIdx]  << std::endl;
    str << "branch_order_:    " << n.branch_order_[n.kIdx]  << std::endl;
    str << "actual_length_:   " << n.actual_length_[n.kIdx]  << std::endl;
    str << "tension_:  " << n.tension_[n.kIdx]  << std::endl;
    str << "spring_constant_: " << n.spring_constant_[n.kIdx]  << std::endl;
    str << "resting_length_:  " << n.resting_length_[n.kIdx]  << std::endl;
    auto mother = n.mother_[n.kIdx].IsNeuron() ? "neuron" : (n.mother_[n.kIdx].IsNeurite() ? "neurite" : "nullptr");
    str << "mother_           " << mother << std::endl;
    return str;
  }

 protected:
   void Copy(const TMostDerived<Backend>& rhs) {
     // TODO adherence
     adherence_[kIdx] = rhs.GetAdherence();
     //  density_
     SetDiameter(rhs.GetDiameter()); // also updates volyume
     x_axis_[kIdx] = rhs.GetXAxis();
     y_axis_[kIdx] = rhs.GetYAxis();
     z_axis_[kIdx] = rhs.GetZAxis();

     spring_axis_[kIdx] = rhs.GetSpringAxis();
     branch_order_[kIdx] = rhs.GetBranchOrder();
     spring_constant_[kIdx] = rhs.GetSpringConstant();
     // TODO what about actual length, tension and resting_length_ ??
   }

   /// collection of biology modules which define the internal behavior
   vec<std::vector<TBiologyModuleVariant>> biology_modules_ = {{}};

   /// Copies biology modules to destination and removes them from
   /// `biology_modules_` if the biology modules are marked for the specific
   /// event. @see BaseBiologyModule
   /// @param[in]  event biology module event - used to determine wether a BM
   ///                   should be copied to destination or removed from
   ///                   from `biology_modules_`
   /// @param[out] destination distination for the new biology modules
   /// @param[in]  skip_removal skip the removal of biology modules. Default
   ///             value is false.
   void BiologyModuleEventHandler(BmEvent event, std::vector<TBiologyModuleVariant>* destination, bool skip_removal = false) {
     CopyVisitor<std::vector<TBiologyModuleVariant>> visitor(event, destination);
     for (auto& module : biology_modules_[kIdx]) {
       visit(visitor, module);
     }

     if (skip_removal) return;

     RemoveVisitor remove_visitor(event);
     for (auto it = biology_modules_[kIdx].begin(); it != biology_modules_[kIdx].end(); ) {
       visit(remove_visitor, *it);
       if (remove_visitor.return_value_) {
         it = biology_modules_[kIdx].erase(it);
       } else {
         ++it;
       }
     }
   }

 private:

   // TODO data members same as in cell
   /// position_ is middle point of cylinder_
   /// mass_location_ is distal end of the cylinder
   vec<std::array<double, 3>> mass_location_ = {{0.0, 0.0, 0.0}};
   vec<double> volume_;
   vec<double> diameter_ = {{ Param::kNeuriteDefaultDiameter }};
   vec<double> adherence_;
   /// First axis of the local coordinate system equal to cylinder axis
   vec<array<double, 3>> x_axis_ = {{1.0, 0.0, 0.0}};
   /// Second axis of the local coordinate system.
   vec<array<double, 3>> y_axis_ = {{0.0, 1.0, 0.0}};
   /// Third axis of the local coordinate system.
   vec<array<double, 3>> z_axis_ = {{0.0, 0.0, 1.0}};
   /// Grid box index
   vec<uint64_t> box_idx_;


  vec<bool> is_axon_ = {{false}};

  /// Parent node in the neuron tree structure can be a Neurite segment
  /// or cell body
  vec<NeuriteOrNeuron> mother_ = {{}};

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

  /// from the attachment point to the mass location
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
  MostDerivedSoPtr InsertProximalCylinder();

  // TODO documentation, TODO rename function
  /// Divides the PhysicalCylinder into two PhysicalCylinders (in fact, into two instances of the derived class).
  /// The one in which the method is called becomes the distal half, and it's length is reduced.
  /// A new PhysicalCylinder is instantiated and becomes the proximal part (=the mother). All characteristics are transmitted
  /// @param distalPortion the fraction of the total old length devoted to the distal half (should be between 0 and 1).
  MostDerivedSoPtr InsertProximalCylinder(double distal_portion);

  // TODO rename function
  /// Merges two neurite elements together. The one in which the method is called phagocytes it's mother.
  void RemoveProximalCylinder();

  // TODO rename function
  MostDerivedSoPtr ExtendSideCylinder(double length, const std::array<double, 3>& direction);
};

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
BDM_SO_DEFINE(inline void NeuriteExt)::RetractTerminalEnd(double speed) {
    // check if is a terminal branch
    if (!daughter_left_[kIdx].IsNullPtr()) {
      return;
    }
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
      spring_axis_[kIdx] = Matrix::ScalarMult(factor, spring_axis_[kIdx]);

      mass_location_[kIdx] = Matrix::Add(mother_[kIdx].OriginOf(Base::GetElementIdx()), spring_axis_[kIdx]);
      UpdateVolume();  // and update concentration of internal stuff.
    } else if (mother_[kIdx].IsNeurite() && mother_[kIdx].GetDaughterRight().IsNullPtr()) {

      // if actual_length_ < length and mother is a PhysicalCylinder with no other daughter : merge with mother
      RemoveProximalCylinder();  // also updates volume_...
      RetractTerminalEnd(speed / Param::simulation_time_step_);
    } else {
      // if mother is cylinder with other daughter or is not a cylinder : disappear.
      mother_[kIdx].RemoveDaughter(GetSoPtr());
      RemoveFromSimulation();

      mother_[kIdx].UpdateDependentPhysicalVariables();
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

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr NeuriteExt)::Branch(double new_branch_diameter, const std::array<double, 3>& direction) {
  // create a new neurite element for side branch
  double length = 1.0;  // TODO hard coded value

  // we first split this neurite segment into two pieces
  auto proximal_ns = InsertProximalCylinder().Get();

  // then append a "daughter right" between the two
  auto new_branch_soptr = proximal_ns.ExtendSideCylinder(length, direction);
  auto new_branch = new_branch_soptr.Get();

  // making the branching at physicalObject level
  new_branch.SetDiameter(diameter_[kIdx]);
  new_branch.SetBranchOrder(branch_order_[kIdx] + 1);
  // TODO : Caution : doesn't change the value distally on the main branch

  std::vector<TBiologyModuleVariant> branch_biology_modules;
  BiologyModuleEventHandler(gNeuriteBifurcation, &branch_biology_modules);
  new_branch.SetBiologyModules(std::move(branch_biology_modules));

  return new_branch_soptr;
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr NeuriteExt)::Branch(const std::array<double, 3>& direction) {
  return Branch(diameter_[kIdx], direction);
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr NeuriteExt)::Branch(double diameter) {
  auto rand_noise = gRandom.NextNoise(0.1);
  auto growth_direction = Math::Perp3(Matrix::Add(GetUnitaryAxisDirectionVector(), rand_noise),
                                        gRandom.NextDouble());
  growth_direction = Math::Normalize(growth_direction);
  return Branch(diameter, growth_direction);
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr NeuriteExt)::Branch() {
  double branch_diameter = diameter_[kIdx];
  auto rand_noise = gRandom.NextNoise(0.1);
  auto growth_direction = Math::Perp3(Matrix::Add(GetUnitaryAxisDirectionVector(), rand_noise),
                                        gRandom.NextDouble());
  return Branch(branch_diameter, growth_direction);
}

BDM_SO_DEFINE(inline bool NeuriteExt)::BifurcationPermitted() const {
  return (daughter_left_[kIdx].IsNullPtr() && actual_length_[kIdx] > Param::kNeuriteMinimalBifurcationLength);
}

BDM_SO_DEFINE(inline std::array<typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr, 2> NeuriteExt)::Bifurcate() {
  // initial default length :
  double l = Param::kNeuriteDefaultActualLength;
  // diameters :
  double d = diameter_[kIdx];
  // direction : (60 degrees between branches)
  double random = gRandom.NextDouble();
  auto perp_plane = Math::Perp3(spring_axis_[kIdx], random);
  double angle_between_branches = Math::kPi / 3.0;
  auto direction_1 = Math::RotAroundAxis(spring_axis_[kIdx], angle_between_branches * 0.5, perp_plane);
  auto direction_2 = Math::RotAroundAxis(spring_axis_[kIdx], -angle_between_branches * 0.5, perp_plane);

  return Bifurcate(l, d, d, direction_1, direction_2);
}

BDM_SO_DEFINE(inline std::array<typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr, 2> NeuriteExt)::Bifurcate(const std::array<double, 3>& direction_1,
                                         const std::array<double, 3>& direction_2) {
   // initial default length :
   double l = Param::kNeuriteDefaultActualLength;
   // diameters :
   double d = diameter_[kIdx];
   return Bifurcate(l, d, d, direction_1, direction_2);
}

BDM_SO_DEFINE(inline std::array<typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr, 2> NeuriteExt)::Bifurcate(double length, double diameter_1, double diameter_2,
                                         const std::array<double, 3>& direction_1,
                                         const std::array<double, 3>& direction_2) {
    // 1) physical bifurcation
    // check it is a terminal branch
      if (!daughter_left_[kIdx].IsNullPtr()) {
        Fatal("Neurites", "Bifurcation only allowed on a terminal neurite segment");
      }
      auto new_branch_l = Rm()->template New<MostDerived>();
      auto new_branch_r = Rm()->template New<MostDerived>();
      // create the cylinders
      new_branch_l.Copy(*static_cast<TMostDerived<Backend>*>(this));
      new_branch_r.Copy(*static_cast<TMostDerived<Backend>*>(this));
      // set family relations
      daughter_left_[kIdx] = new_branch_l.GetSoPtr();
      new_branch_l.SetMother(GetSoPtr());
      daughter_right_[kIdx] = new_branch_r.GetSoPtr();
      new_branch_r.SetMother(GetSoPtr());

      // check that the directions are not pointing backwards
      auto dir_1 = direction_1;  // todo avoid cpy
      auto dir_2 = direction_2;
      if (Math::AngleRadian(spring_axis_[kIdx], direction_1) > Math::kPi / 2.0) {
        auto proj = Math::ProjectionOnto(direction_1, spring_axis_[kIdx]);
        proj = Matrix::ScalarMult(-1, proj);
        dir_1 = Matrix::Add(direction_1, proj);
      }
      if (Math::AngleRadian(spring_axis_[kIdx], direction_2) > Math::kPi / 2.0) {
        auto proj = Math::ProjectionOnto(direction_2, spring_axis_[kIdx]);
        proj = Matrix::ScalarMult(-1, proj);
        dir_2 = Matrix::Add(direction_2, proj);
      }

      // mass location and spring axis
      new_branch_l.SetSpringAxis(Matrix::ScalarMult(length, Math::Normalize(dir_1)));
      new_branch_l.SetMassLocation(Matrix::Add(mass_location_[kIdx], new_branch_l.GetSpringAxis()));
      new_branch_l.UpdateLocalCoordinateAxis();  // (important so that x_axis_ is correct)

      new_branch_r.SetSpringAxis(Matrix::ScalarMult(length, Math::Normalize(dir_2)));
      new_branch_r.SetMassLocation(Matrix::Add(mass_location_[kIdx], new_branch_r.GetSpringAxis()));
      new_branch_r.UpdateLocalCoordinateAxis();

      // physics of tension :
      new_branch_l.SetActualLength(length);
      new_branch_r.SetActualLength(length);
      new_branch_r.SetRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
      new_branch_l.SetRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);

      // set local coordinate axis in the new branches
      // TODO again?? alreay done a few lines up
      new_branch_l.UpdateLocalCoordinateAxis();
      new_branch_r.UpdateLocalCoordinateAxis();

      // i'm scheduled to run physics next time :
      // (the daughters automatically are too, because they are new PhysicalObjects)
      // setOnTheSchedulerListForPhysicalObjects(true);

      new_branch_l.UpdateDependentPhysicalVariables();
      new_branch_r.UpdateDependentPhysicalVariables();


    // 2) creating the first daughter branch
    new_branch_l.SetDiameter(diameter_1);
    new_branch_l.SetBranchOrder(branch_order_[kIdx] + 1);

    // 3) the second one
    new_branch_r.SetDiameter(diameter_2);
    new_branch_r.SetBranchOrder(branch_order_[kIdx] + 1);

    // 4) the biological modules :
    std::vector<TBiologyModuleVariant> biology_modules_l;
    BiologyModuleEventHandler(gNeuriteBifurcation, &biology_modules_l);
    new_branch_l.SetBiologyModules(std::move(biology_modules_l));

    std::vector<TBiologyModuleVariant> biology_modules_r;
    BiologyModuleEventHandler(gNeuriteBifurcation, &biology_modules_r, true);
    new_branch_r.SetBiologyModules(std::move(biology_modules_r));

    return {new_branch_l.GetSoPtr(), new_branch_r.GetSoPtr()};
}


BDM_SO_DEFINE(inline void NeuriteExt)::RemoveDaughter(const typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr& daughter) {
  // If there is another daughter than the one we want to remove,
  // we have to be sure that it will be the daughterLeft->
  if (daughter == daughter_right_[kIdx]) {
    daughter_right_[kIdx] = MostDerivedSoPtr();
    return;
  }

  if (daughter == daughter_left_[kIdx]) {
    daughter_left_[kIdx] = daughter_right_[kIdx];
    daughter_right_[kIdx] = MostDerivedSoPtr();
    return;
  }
  Fatal("Neurite", "Given object is not a daughter!");
}

BDM_SO_DEFINE(inline void NeuriteExt)::UpdateRelative(const NeuriteOrNeuron& old_relative,
                                                      const NeuriteOrNeuron& new_relative) {
  if (old_relative == mother_[kIdx]) {
    mother_[kIdx] = new_relative;
  } else {
    auto old_neurite = old_relative.GetNeuriteSoPtr();
    auto new_neurite = new_relative.GetNeuriteSoPtr();
    if (old_neurite == daughter_left_[kIdx]) {
      daughter_left_[kIdx] = new_neurite;
    } else if (old_neurite == daughter_right_[kIdx]) {
      daughter_right_[kIdx] = new_neurite;
    }
  }
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::ForceTransmittedFromDaugtherToMother(const NeuriteOrNeuron& mother) {
  if(mother_[kIdx] != mother) {
    Fatal("Neurite", "Given object is not the mother!");
  }

  // The inner tension is added to the external force that was computed earlier.
  // (The reason for dividing by the actualLength is to normalize the direction : T = T * axis/ (axis length)
  double factor = tension_[kIdx] / actual_length_[kIdx];
  if (factor < 0) {
    factor = 0;
  }
  return Matrix::Add(Matrix::ScalarMult(factor, spring_axis_[kIdx]), force_to_transmit_to_proximal_mass_[kIdx]);
}

BDM_SO_DEFINE(inline void NeuriteExt)::RunDiscretization() {
  if (actual_length_[kIdx] > Param::kNeuriteMaxLength) {
    if (daughter_left_[kIdx].IsNullPtr()) { // if terminal branch :
      std::cout << "InsertProximalCylinder 0.1" << std::endl;
      InsertProximalCylinder(0.1);
    } else if (mother_[kIdx].IsNeuron()) {  // if initial branch :
      std::cout << "InsertProximalCylinder 0.9" << std::endl;
      InsertProximalCylinder(0.9);
    } else {
      std::cout << "InsertProximalCylinder 0.5" << std::endl;
      InsertProximalCylinder(0.5);
    }
  } else if (actual_length_[kIdx] < Param::kNeuriteMinLength && mother_[kIdx].IsNeurite()
      && mother_[kIdx].GetRestingLength() < Param::kNeuriteMaxLength - resting_length_[kIdx] - 1
      && mother_[kIdx].GetDaughterRight().IsNullPtr() && !daughter_left_[kIdx].IsNullPtr()) {
    // if the previous branch is removed, we first remove its associated NeuriteElement
    std::cout << "InsertProximalCylinder removeYourself" << std::endl;
    mother_[kIdx].RemoveYourself();
    // then we remove it
    RemoveProximalCylinder();
    // TODO LB: what about ourselves??
  }
}

BDM_SO_DEFINE(inline void NeuriteExt)::MovePointMass(double speed, const std::array<double, 3>& direction) {
  // check if is a terminal branch
  if (!daughter_left_[kIdx].IsNullPtr()) {
    return;
  }

  // scaling for integration step
  double length = speed * Param::simulation_time_step_;
  auto displacement = Matrix::ScalarMult(length, Math::Normalize(direction));
  auto new_mass_location = Matrix::Add(displacement, mass_location_[kIdx]);
  // here I have to define the actual length ..........
  // auto& relative_pos = mother_[kIdx].GetPosition();
  auto relative_ml = mother_[kIdx].OriginOf(Base::GetElementIdx()); // TODO change to auto&&
  std::cout << "direction    " << direction[0] << ", " << direction[1] << ", " << direction[2] << std::endl;
  std::cout << "spring_axis_ " << spring_axis_[kIdx][0] << ", " << spring_axis_[kIdx][1] << ", " << spring_axis_[kIdx][2] << std::endl;
  spring_axis_[kIdx] = Matrix::Subtract(new_mass_location, relative_ml);
  mass_location_[kIdx] = new_mass_location;
  actual_length_[kIdx] = std::sqrt(Matrix::Dot(spring_axis_[kIdx], spring_axis_[kIdx]));
  // process of elongation : setting tension to 0 increases the resting length :
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
}

BDM_SO_DEFINE(inline void NeuriteExt)::ChangeDiameter(double speed) {
  //scaling for integration step
  double dD = speed * Param::simulation_time_step_;
  diameter_[kIdx] += dD;
  UpdateVolume();
}

BDM_SO_DEFINE(template <typename TGrid>
inline std::array<double, 3> NeuriteExt)::CalculateDisplacement(TGrid* grid, double squared_radius) {
  // decide first if we have to split or fuse this cylinder. Usually only
  // terminal branches (growth cone) do
  if (daughter_left_[kIdx].IsNullPtr()) {
    RunDiscretization();
  }

  // in case we don't move, we won't run physics the next time :
  // setOnTheSchedulerListForPhysicalObjects(false);

  double h = Param::simulation_time_step_; // TODO rename h
  std::array<double, 3> force_on_my_point_mass { 0, 0, 0 };
  std::array<double, 3> force_on_my_mothers_point_mass { 0, 0, 0 };

  // 1) Spring force
  //   Only the spring of this cylinder. The daughters spring also act on this
  //    mass, but they are treated in point (2)
  double factor = -tension_[kIdx] / actual_length_[kIdx];  // the minus sign is important because the spring axis goes in the opposite direction
  force_on_my_point_mass = Matrix::Add(force_on_my_point_mass, Matrix::ScalarMult(factor, spring_axis_[kIdx]));

  // 2) Force transmitted by daugthers (if they exist) ----------------------------------
  if (!daughter_left_[kIdx].IsNullPtr()) {
    auto force_from_daughter = daughter_left_[kIdx].Get().ForceTransmittedFromDaugtherToMother(GetSoPtr());
    force_on_my_point_mass = Matrix::Add(force_on_my_point_mass, force_from_daughter);
  }
  if (!daughter_right_[kIdx].IsNullPtr()) {
    auto force_from_daughter = daughter_right_[kIdx].Get().ForceTransmittedFromDaugtherToMother(GetSoPtr());
    force_on_my_point_mass = Matrix::Add(force_on_my_point_mass, force_from_daughter);
  }

  // 3) Object avoidance force -----------------------------------------------------------
  //  (We check for every neighbor object if they touch us, i.e. push us away)
  auto calculate_neighbor_forces = [this,&force_on_my_point_mass, &force_on_my_mothers_point_mass](auto&& neighbor,
                                       SoHandle neighbor_handle) {
    using NeighborBackend = typename std::decay<decltype(neighbor)>::type::Backend;
    using NeighborNeurite = TMostDerived<NeighborBackend>;
    using NeighborNeuron = typename TNeuron::template Self<NeighborBackend>;

    // TODO(lukas) once we switch to C++17 use if constexpr.
    // As a consequence the reinterpret_cast won't be needed anymore.

    // if neighbor is a Neurite
    if (std::is_same<NeighborNeurite, std::decay_t<decltype(neighbor)>>::value) {
      auto n_soptr = reinterpret_cast<NeighborNeurite*>(&neighbor)->GetSoPtr();
      // if it is a direct relative, or sister branch, we don't take it into account
      if (n_soptr == this->GetDaughterLeft() ||
          n_soptr == this->GetDaughterRight() ||
          (this->GetMother().IsNeurite() && this->GetMother().GetNeuriteSoPtr() == n_soptr) ||
          n_soptr.Get().GetMother() == this->GetMother()) {
        return;
      }
    } else if (std::is_same<NeighborNeuron, std::decay_t<decltype(neighbor)>>::value) {
      // if neighbor is Neuron
      // if it is a direct relative, we don't take it into account
      auto n_soptr = reinterpret_cast<NeighborNeuron*>(&neighbor)->GetSoPtr();
      if(this->GetMother().IsNeuron() && this->GetMother().GetNeuronSoPtr() == n_soptr) {
        return;
      }
    }

    DefaultForce force;
    std::array<double, 4> force_from_neighbor = force.GetForce(this, &neighbor);

    // 1) "artificial force" to maintain the sphere in the ecm simulation boundaries--------
    // TODO remove this; this is done in dispalcement op
    // if (ecm_->getArtificialWallForCylinders()) {
    //   auto force_from_artificial_wall = ecm_->forceFromArtificialWall(mass_location_, diameter_ * 0.5);
    //   force_on_my_point_mass[0] += force_from_artificial_wall[0];
    //   force_on_my_point_mass[1] += force_from_artificial_wall[1];
    //   force_on_my_point_mass[2] += force_from_artificial_wall[2];
    // }

    if (std::abs(force_from_neighbor[3]) < 1E-10) {  // TODO hard coded value
      // (if all the force is transmitted to the (distal end) point mass : )
      force_on_my_point_mass[0] += force_from_neighbor[0];
      force_on_my_point_mass[1] += force_from_neighbor[1];
      force_on_my_point_mass[2] += force_from_neighbor[2];
    } else {
      // (if there is a part transmitted to the proximal end : )
      double part_for_point_mass = 1.0 - force_from_neighbor[3];
      force_on_my_point_mass[0] += force_from_neighbor[0] * part_for_point_mass;
      force_on_my_point_mass[1] += force_from_neighbor[1] * part_for_point_mass;
      force_on_my_point_mass[2] += force_from_neighbor[2] * part_for_point_mass;
      force_on_my_mothers_point_mass[0] += force_from_neighbor[0] * force_from_neighbor[3];
      force_on_my_mothers_point_mass[1] += force_from_neighbor[1] * force_from_neighbor[3];
      force_on_my_mothers_point_mass[2] += force_from_neighbor[2] * force_from_neighbor[3];
    }
  };

  grid->ForEachNeighborWithinRadius(calculate_neighbor_forces, *this,
                                    GetSoHandle(), squared_radius);

  bool anti_kink = false;
  // TEST : anti-kink
  if (anti_kink) {
    double KK = 5;
    if (!daughter_left_[kIdx].IsNullPtr() && daughter_right_[kIdx].IsNullPtr()) {
      if (!daughter_left_[kIdx].Get().GetDaughterLeft().IsNullPtr()) {
        auto downstream = daughter_left_[kIdx].Get().GetDaughterLeft().Get();
        double rresting = daughter_left_[kIdx].Get().GetRestingLength() + downstream.GetRestingLength();
        auto down_to_me = Matrix::Subtract(GetMassLocation(), downstream.GetMassLocation());
        double aactual = Math::Norm(down_to_me);

        force_on_my_point_mass = Matrix::Add(
            force_on_my_point_mass, Matrix::ScalarMult(KK * (rresting - aactual), Math::Normalize(down_to_me)));
      }
    }

    if (!daughter_left_[kIdx].IsNullPtr() && mother_[kIdx].IsNeurite()) {
      // TODO rename mother_cyl
      auto mother_cyl = mother_[kIdx].GetNeuriteSoPtr().Get();
      double rresting = GetRestingLength() + mother_cyl.GetRestingLength();
      auto down_to_me = Matrix::Subtract(GetMassLocation(), mother_cyl.ProximalEnd());
      double aactual = Math::Norm(down_to_me);

      force_on_my_point_mass = Matrix::Add(
          force_on_my_point_mass, Matrix::ScalarMult(KK * (rresting - aactual), Math::Normalize(down_to_me)));
    }
  }

  // 5) define the force that will be transmitted to the mother
  force_to_transmit_to_proximal_mass_[kIdx] = force_on_my_mothers_point_mass;
  // 6) Compute the movement of this neurite elicited by the resultant force----------------
  //  6.0) In case we display the force
  // TODO total_force_last_time_step_ is never read
  // total_force_last_time_step_[kIdx][0] = force_on_my_point_mass[0];
  // total_force_last_time_step_[kIdx][1] = force_on_my_point_mass[1];
  // total_force_last_time_step_[kIdx][2] = force_on_my_point_mass[2];
  // total_force_last_time_step_[kIdx][3] = 1;
  //  6.1) Define movement scale
  // FIXME
  // double h_over_m = h / GetMass();
  double h_over_m = 1.0;
  double force_norm = Math::Norm(force_on_my_point_mass);
  //  6.2) If is F not strong enough -> no movements
  if (force_norm < adherence_[kIdx]) {
    // TODO total_force_last_time_step_ is never read
    // total_force_last_time_step_[kIdx][3] = -1;
    return {0, 0, 0};
  }
  // So, what follows is only executed if we do actually move :

  //  6.3) Since there's going be a move, we calculate it
  auto displacement = Matrix::ScalarMult(h_over_m, force_on_my_point_mass);
  double displacement_norm = force_norm * h_over_m;

  //  6.4) There is an upper bound for the movement.
  if (displacement_norm > Param::simulation_max_displacement_) {
    displacement = Matrix::ScalarMult(Param::simulation_max_displacement_ / displacement_norm, displacement);
  }

  return displacement;
}

BDM_SO_DEFINE(inline void NeuriteExt)::ApplyDisplacement(const std::array<double, 3>& displacement) {
  // move of our mass
  SetMassLocation(Matrix::Add(GetMassLocation(), displacement));
  // Recompute length, tension and re-center the computation node, and redefine axis
  UpdateDependentPhysicalVariables();
  UpdateLocalCoordinateAxis();

  // FIXME this whole block might be superfluous - ApplyDisplacement is called
  // For the relatives: recompute the lenght, tension etc. (why for mother? have to think about that)
  if (!daughter_left_[kIdx].IsNullPtr()){
    auto left = daughter_left_[kIdx].Get();
    // FIXME this is problematic for the distributed version. it modifies a "neightbor"
    left.UpdateDependentPhysicalVariables();
    left.UpdateLocalCoordinateAxis();
  }
  if (!daughter_right_[kIdx].IsNullPtr()){
    // FIXME this is problematic for the distributed version. it modifies a "neightbor"
    auto right = daughter_right_[kIdx].Get();
    right.UpdateDependentPhysicalVariables();
    right.UpdateLocalCoordinateAxis();
  }
}

BDM_SO_DEFINE(inline void NeuriteExt)::UpdateVolume() {
  volume_[kIdx] = Math::kPi / 4 * diameter_[kIdx] * diameter_[kIdx] * actual_length_[kIdx];
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

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::NeuriteOrNeuron NeuriteExt)::GetMother() {
  return mother_[kIdx];
}

BDM_SO_DEFINE(inline void NeuriteExt)::SetMother(const NeuriteOrNeuron& mother) {
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
  double factor = 1.0 / actual_length_[kIdx];
  return Matrix::ScalarMult(factor, spring_axis_[kIdx]);
}

BDM_SO_DEFINE(inline bool NeuriteExt)::IsTerminal() const {
  return daughter_left_[kIdx].IsNullPtr();
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuriteExt)::ProximalEnd() const {
  return Matrix::Subtract(mass_location_[kIdx], spring_axis_[kIdx]);
}

BDM_SO_DEFINE(inline const std::array<double, 3>& NeuriteExt)::DistalEnd() const {
  return mass_location_[kIdx];
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
  auto relative_ml = mother_[kIdx].OriginOf(Base::GetElementIdx());
  spring_axis_[kIdx] = Matrix::Subtract(mass_location_[kIdx], relative_ml);
  actual_length_[kIdx] = std::sqrt(Matrix::Dot(spring_axis_[kIdx], spring_axis_[kIdx]));
  tension_[kIdx] = spring_constant_[kIdx] * (actual_length_[kIdx] - resting_length_[kIdx]) / resting_length_[kIdx];
  UpdateVolume();
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr NeuriteExt)::InsertProximalCylinder() {
  return InsertProximalCylinder(0.5);
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr NeuriteExt)::InsertProximalCylinder(double distal_portion) {
  auto new_neurite_element = Rm()->template New<MostDerived>();

  // TODO reformulate to mass_location_
  auto new_position = Matrix::Subtract(mass_location_[kIdx], Matrix::ScalarMult(distal_portion, spring_axis_[kIdx]));

  new_neurite_element.SetPosition(new_position);
  new_neurite_element.Copy(*static_cast<TMostDerived<Backend>*>(this));


  // family relations
  mother_[kIdx].UpdateRelative(NeuriteOrNeuron(GetSoPtr()), NeuriteOrNeuron(new_neurite_element.GetSoPtr()));
  new_neurite_element.SetMother(mother_[kIdx]);
  SetMother(new_neurite_element.GetSoPtr());
  new_neurite_element.SetDaughterLeft(GetSoPtr());
  // physics
  new_neurite_element.SetRestingLength((1 - distal_portion) * resting_length_[kIdx]);
  resting_length_[kIdx] *= distal_portion;

  UpdateDependentPhysicalVariables();
  new_neurite_element.UpdateDependentPhysicalVariables();
  // UpdateLocalCoordinateAxis has to come after UpdateDepend...
  new_neurite_element.UpdateLocalCoordinateAxis();

  std::vector<TBiologyModuleVariant> new_biology_modules;
  // TODO what about gNeuriteSideCylinderExtension ??
  BiologyModuleEventHandler(gNeuriteElongation, &new_biology_modules);
  new_neurite_element.SetBiologyModules(std::move(new_biology_modules));

  // FIXME what about data members in subclasses
  return new_neurite_element.GetSoPtr();
}

BDM_SO_DEFINE(inline typename NeuriteExt<TCompileTimeParam, TDerived, TBase>::MostDerivedSoPtr NeuriteExt)::ExtendSideCylinder(double length, const std::array<double, 3>& direction) {

  auto new_branch = Rm()->template New<MostDerived>();
  new_branch.Copy(*static_cast<TMostDerived<Backend>*>(this));

  auto dir = direction;
  double angle_with_side_branch = Math::AngleRadian(spring_axis_[kIdx], direction);
  if (angle_with_side_branch < 0.78 || angle_with_side_branch > 2.35) {  // 45-135 degrees
    auto p = Math::CrossProduct(spring_axis_[kIdx], direction);
    p = Math::CrossProduct(p, spring_axis_[kIdx]);
    dir = Matrix::Add(Math::Normalize(direction), Math::Normalize(p));
  }
  // location of mass and computation center
  auto new_spring_axis = Matrix::ScalarMult(length, Math::Normalize(direction));
  new_branch.SetMassLocation(Matrix::Add(mass_location_[kIdx], new_spring_axis));
  new_branch.SetSpringAxis(new_spring_axis);
  // physics
  new_branch.SetActualLength(length);
  new_branch.SetRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
  new_branch.SetDiameter(Param::kNeuriteDefaultDiameter);
  new_branch.UpdateLocalCoordinateAxis();
  // family relations
  new_branch.SetMother(GetSoPtr());
  daughter_right_[kIdx] = new_branch.GetSoPtr();

  // correct physical values (has to be after family relations and SON assignement).
  new_branch.UpdateDependentPhysicalVariables();

  return new_branch.GetSoPtr();
}

BDM_SO_DEFINE(inline void NeuriteExt)::RemoveProximalCylinder() {
  // The mother is removed if (a) it is a neurite segment and (b) it has no other daughter than
  if (!mother_[kIdx].IsNeurite() || !mother_[kIdx].GetDaughterRight().IsNullPtr()) {
    return;
  }
  // The guy we gonna remove
  // TODO rename to neurite (no cylinder)
  auto proximal_cylinder = mother_[kIdx].GetNeuriteSoPtr().Get();

  // Re-organisation of the PhysicalObject tree structure: by-passing proximalCylinder
  proximal_cylinder.GetMother().UpdateRelative(mother_[kIdx], NeuriteOrNeuron(GetSoPtr()));
  SetMother(mother_[kIdx].GetMother());

  // Keeping the same tension :
  // (we don't use updateDependentPhysicalVariables(), because we have tension and want to
  // compute restingLength, and not the opposite...)
  // T = k*(A-R)/R --> R = k*A/(T+K)
  spring_axis_[kIdx] = Matrix::Subtract(mass_location_[kIdx], mother_[kIdx].OriginOf(Base::GetElementIdx()));
  actual_length_[kIdx] = Math::Norm(spring_axis_[kIdx]);
  resting_length_[kIdx] = spring_constant_[kIdx] * actual_length_[kIdx] / (tension_[kIdx] + spring_constant_[kIdx]);
  // .... and volume_
  UpdateVolume();
  // and local coord
  UpdateLocalCoordinateAxis();

  proximal_cylinder.RemoveFromSimulation();
}

BDM_SO_DEFINE(inline void NeuriteExt)::RemoveYourself() {
  RemoveFromSimulation();
  // FIXME remove this function since it only forwards to `RemoveFromSimulation`
}

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURITE_H_
