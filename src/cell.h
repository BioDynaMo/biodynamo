#ifndef CELL_H_
#define CELL_H_

#include <array>
#include <cmath>
#include <complex>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "backend.h"
#include "biology_module_util.h"
#include "default_force.h"
#include "inline_vector.h"
#include "math_util.h"
#include "matrix.h"
#include "param.h"
#include "simulation_object.h"
#include "simulation_object_util.h"

namespace bdm {

using std::array;

/// Declare new biology module event for cell division
extern const BmEvent gCellDivision;

BDM_SIM_OBJECT(Cell, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(CellExt, 1, position_, tractor_force_,
                        diameter_, volume_, adherence_, density_,
                        biology_modules_, box_idx_);
 public:
  /// Returns the data members that are required to visualize this simulation
  /// object.
  static std::set<std::string> GetRequiredVisDataMembers() {
    return {"position_", "diameter_"};
  }

  using TBiologyModuleVariant = typename TCompileTimeParam::BiologyModules;
  CellExt() : density_{1.0} {}
  explicit CellExt(double diameter) : diameter_(diameter), density_{1.0} {
    UpdateVolume();
  }
  explicit CellExt(const array<double, 3>& position)
      : position_(position), density_{1.0} {}

  virtual ~CellExt() {}

  /// Add a biology module to this cell
  /// @tparam TBiologyModule type of the biology module. Must be in the set of
  ///         types specified in `TBiologyModuleVariant`
  template <typename TBiologyModule>
  void AddBiologyModule(TBiologyModule && module);

  /// Execute all biology modules
  void RunBiologyModules();

  /// Divide the cell. Of the two daughter cells, one is this one (but smaller,
  /// with half GeneSubstances etc.),
  /// and the other one is given as parameter and initialzed accordingly. Both
  /// cells have more or less the same volume,
  /// the axis of division is random.
  /// @param daughter - second daughter cell = scalar instance which will be
  /// initialized in this method
  void Divide(TMostDerived<Scalar>* daughter);

  /// Divide the cell. Of the two daughter cells, one is this one (but smaller,
  /// with half GeneSubstances etc.),
  /// and the other one is given as parameter and initialzed accordingly. The
  /// axis of division is random.
  /// @param daughter second daughter cell = scalar instance which will be
  /// initialized in this method
  /// @param volume_ratio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0
  /// gives equal cells.
  void Divide(TMostDerived<Scalar>* daughter, double volume_ratio);

  /// Divide the cell. Of the two daughter cells, one is this one (but smaller,
  /// with half GeneSubstances etc.),
  /// and the other one is is given as parameter and initialzed accordingly.
  /// @param daughter second daughter cell = scalar instance which will be
  /// initialized in this method
  /// @param axis specifies direction of division
  void Divide(TMostDerived<Scalar>* daughter, const array<double, 3>& axis);

  /// Divide the cell. Of the two daughter cells, one is this one (but smaller,
  /// with half GeneSubstances etc.),
  /// and the other one is instantiated de novo and is returned.
  /// @param daughter second daughter cell = scalar instance which will be
  /// initialized in this method
  /// @param volume_ratio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0
  /// gives equal cells.
  /// @param axis specifies direction of division
  void Divide(TMostDerived<Scalar> * daughter, double volume_ratio,
              const array<double, 3>& axis);

  /// Forwards call to DivideImpl @see DivideImpl
  void Divide(TMostDerived<Scalar> * daughter, double volume_ratio, double phi,
              double theta);

  /// Divide mother cell in two daughter cells\n
  /// When mother cell divides, by definition:\n
  /// 1) the mother cell becomes the 1st daughter cell\n
  /// 2) the new cell becomes the 2nd daughter cell and inherits a equal or
  /// bigger volume than the 1st
  ///    daughter cell, which means that this cell will eventually inherit more
  ///    differentiating factors
  ///    and will be recorded in the left side of the lineage tree.
  ///
  /// @param daughter second daughter cell = scalar instance which will be
  /// initialized in this method
  /// @param volume_ratio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0
  /// gives equal cells.
  /// @param phi azimuthal angle (polar coordinate)
  /// @param theta polar angle (polar coordinate)
  /// @see \link simulation_object_util.h Divide \endlink
  virtual void DivideImpl(TMostDerived<Scalar> * daughter, double volume_ratio,
                          double phi, double theta);

  double GetAdherence() const { return adherence_[kIdx]; }

  double GetDiameter() const { return diameter_[kIdx]; }

  double GetMass() const { return density_[kIdx] * volume_[kIdx]; }

  double GetDensity() const { return density_[kIdx]; }

  const array<double, 3>& GetPosition() const { return position_[kIdx]; }

  double* GetPositionPtr() { return &(position_[0][0]); }
  double* GetDiameterPtr() { return &(diameter_[0]); }

  const array<double, 3>& GetTractorForce() const {
    return tractor_force_[kIdx];
  }

  double GetVolume() const { return volume_[kIdx]; }

  void SetAdherence(double adherence) { adherence_[kIdx] = adherence; }

  void SetDiameter(double diameter) {
    diameter_[kIdx] = diameter;
    UpdateVolume();
  }

  void SetMass(double mass) { density_[kIdx] = mass / volume_[kIdx]; }

  void SetDensity(double density) { density_[kIdx] = density; }

  void SetPosition(const array<double, 3>& position) {
    position_[kIdx] = position;
  }

  void SetCoordinate(size_t d, double value) {
    position_[kIdx][d] = value;
  }

  void SetTractorForce(const array<double, 3>& tractor_force) {
    tractor_force_[kIdx] = tractor_force;
  }

  void ChangeVolume(double speed) {
    // scaling for integration step
    double delta = speed * Param::simulation_time_step_;
    volume_[kIdx] += delta;
    if (volume_[kIdx] < 5.2359877E-7) {
      volume_[kIdx] = 5.2359877E-7;
    }
    UpdateDiameter();
  }

  void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    diameter_[kIdx] = std::cbrt(volume_[kIdx] * 6 / Math::kPi);
  }

  void UpdateVolume() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    volume_[kIdx] = Math::kPi / 6 * std::pow(diameter_[kIdx], 3);
  }

  void UpdatePosition(const array<double, 3>& delta) {
    position_[kIdx][0] += delta[0];
    position_[kIdx][1] += delta[1];
    position_[kIdx][2] += delta[2];
  }

  void GetForceOn(const array<double, 3>& ref_mass_location,
                  double ref_diameter, array<double, 3>* force) const {
    DefaultForce default_force;
    // TODO(lukas) think about default values in config file
    double iof_coefficient = 0.15;

    default_force.ForceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, position_[kIdx],
                                      diameter_[kIdx], iof_coefficient, force);
  }

  uint64_t GetBoxIdx() const { return box_idx_[kIdx]; }

  void SetBoxIdx(uint64_t idx) { box_idx_[kIdx] = idx; }

 protected:
  /// Returns the position in the polar coordinate system (cylindrical or
  /// spherical) of a point expressed in global cartesian coordinates
  /// ([1,0,0],[0,1,0],[0,0,1]).
  /// @param coord: position in absolute coordinates - [x,y,z] cartesian values
  /// @return the position in local coordinates
  array<double, 3> TransformCoordinatesGlobalToPolar(
      const array<double, 3>& coord) const;

  vec<array<double, 3>> position_;
  vec<array<double, 3>> tractor_force_;
  vec<double> diameter_;
  vec<double> volume_;
  vec<double> adherence_;
  vec<double> density_;

  /// First axis of the local coordinate system.
  static constexpr array<double, 3> kXAxis = {{1.0, 0.0, 0.0}};
  /// Second axis of the local coordinate system.
  static constexpr array<double, 3> kYAxis = {{0.0, 1.0, 0.0}};
  /// Third axis of the local coordinate system.
  static constexpr array<double, 3> kZAxis = {{0.0, 0.0, 1.0}};

  /// collection of biology modules which define the internal behavior
  vec<std::vector<TBiologyModuleVariant>> biology_modules_;

  /// Grid box index
  vec<uint64_t> box_idx_;
};

// ----------------------------------------------------------------------------
// Implementation -------------------------------------------------------------
BDM_SO_DEFINE(constexpr array<double, 3> CellExt)::kXAxis;
BDM_SO_DEFINE(constexpr array<double, 3> CellExt)::kYAxis;
BDM_SO_DEFINE(constexpr array<double, 3> CellExt)::kZAxis;

BDM_SO_DEFINE(template <typename TBiologyModule>
              inline void CellExt)::AddBiologyModule(TBiologyModule&& module) {
  biology_modules_[kIdx].emplace_back(module);
}


BDM_SO_DEFINE(inline void CellExt)::RunBiologyModules() {
  RunVisitor<TMostDerived<Backend>> visitor(static_cast<TMostDerived<Backend>*>(this));
  for (auto& module : biology_modules_[kIdx]) {
    visit(visitor, module);
  }
}

BDM_SO_DEFINE(inline void CellExt)::Divide(TMostDerived<Scalar>* daughter) {
  Divide(daughter, 0.9 + 0.2 * gRandom.NextDouble());
}

BDM_SO_DEFINE(inline void CellExt)::Divide(TMostDerived<Scalar>* daughter,
                                           double volume_ratio) {
  // find random point on sphere (based on :
  // http://mathworld.wolfram.com/SpherePointPicking.html)
  double theta = 2 * Math::kPi * gRandom.NextDouble();
  double phi = std::acos(2 * gRandom.NextDouble() - 1);
  DivideImpl(daughter, volume_ratio, phi, theta);
}

BDM_SO_DEFINE(inline void CellExt)::Divide(TMostDerived<Scalar>* daughter,
                                           const array<double, 3>& axis) {
  auto polarcoord =
      TransformCoordinatesGlobalToPolar(Matrix::Add(axis, position_[kIdx]));
  DivideImpl(daughter, 0.9 + 0.2 * gRandom.NextDouble(), polarcoord[1],
             polarcoord[2]);
}

BDM_SO_DEFINE(inline void CellExt)::Divide(TMostDerived<Scalar>* daughter,
                                           double volume_ratio,
                                           const array<double, 3>& axis) {
  auto polarcoord =
      TransformCoordinatesGlobalToPolar(Matrix::Add(axis, position_[kIdx]));
  DivideImpl(daughter, volume_ratio, polarcoord[1], polarcoord[2]);
}

BDM_SO_DEFINE(inline void CellExt)::Divide(TMostDerived<Scalar>* daughter,
                                           double volume_ratio, double phi,
                                           double theta) {
  DivideImpl(daughter, volume_ratio, phi, theta);
}

BDM_SO_DEFINE(inline void CellExt)::DivideImpl(TMostDerived<Scalar>* daughter,
                                               double volume_ratio, double phi,
                                               double theta) {
  // A) Defining some values
  // ..................................................................
  // defining the two radii s.t total volume is conserved
  // * radius^3 = r1^3 + r2^3 ;
  // * volume_ratio = r2^3 / r1^3
  double radius = diameter_[kIdx] * 0.5;
  double r1 = radius / std::pow(1.0 + volume_ratio, 1.0 / 3.0);
  double r2 = radius / std::pow(1.0 + 1 / volume_ratio, 1.0 / 3.0);

  // define an axis for division (along which the nuclei will move)
  double x_coord = std::cos(theta) * std::sin(phi);
  double y_coord = std::sin(theta) * std::sin(phi);
  double z_coord = std::cos(phi);
  double total_length_of_displacement = radius / 4.0;
  array<double, 3> axis_of_division{
      total_length_of_displacement *
          (x_coord * kXAxis[0] + y_coord * kYAxis[0] + z_coord * kZAxis[0]),
      total_length_of_displacement *
          (x_coord * kXAxis[1] + y_coord * kYAxis[1] + z_coord * kZAxis[1]),
      total_length_of_displacement *
          (x_coord * kXAxis[2] + y_coord * kYAxis[2] + z_coord * kZAxis[2])};

  // two equations for the center displacement :
  //  1) d2/d1= v2/v1 = volume_ratio (each sphere is shifted inver.
  //  proportionally to its volume)
  //  2) d1 + d2 = TOTAL_LENGTH_OF_DISPLACEMENT
  double d_2 = total_length_of_displacement / (volume_ratio + 1);
  double d_1 = total_length_of_displacement - d_2;

  // B) Instantiating a new sphere = 2nd daughter
  daughter->adherence_[0] = adherence_[kIdx];
  daughter->density_[0] = density_[kIdx];

  daughter->diameter_[0] = r2 * 2;
  daughter->UpdateVolume();

  // position
  array<double, 3> new_position{position_[kIdx][0] + d_2 * axis_of_division[0],
                                position_[kIdx][1] + d_2 * axis_of_division[1],
                                position_[kIdx][2] + d_2 * axis_of_division[2]};
  daughter->position_[0] = new_position;

  CopyVisitor<std::vector<TBiologyModuleVariant>> visitor(
      gCellDivision, &(daughter->biology_modules_[0]));
  for (auto& module : biology_modules_[kIdx]) {
    visit(visitor, module);
  }

  // E) This sphere becomes the 1st daughter
  // move these cells on opposite direction
  position_[kIdx][0] -= d_1 * axis_of_division[0];
  position_[kIdx][1] -= d_1 * axis_of_division[1];
  position_[kIdx][2] -= d_1 * axis_of_division[2];

  // F) change properties of this cell
  diameter_[kIdx] = r1 * 2;
  UpdateVolume();

  daughter->box_idx_[0] = box_idx_[kIdx];

  // G) TODO(lukas) Copy the intracellular and membrane bound Substances
}

BDM_SO_DEFINE(array<double, 3> CellExt)::TransformCoordinatesGlobalToPolar(
    const array<double, 3>& pos) const {
  auto vector_to_point = Matrix::Subtract(pos, position_[kIdx]);
  array<double, 3> local_cartesian{Matrix::Dot(kXAxis, vector_to_point),
                                   Matrix::Dot(kYAxis, vector_to_point),
                                   Matrix::Dot(kZAxis, vector_to_point)};
  return {std::sqrt(local_cartesian[0] * local_cartesian[0] +
                    local_cartesian[1] * local_cartesian[1] +
                    local_cartesian[2] * local_cartesian[2]),
          std::atan2(std::sqrt(local_cartesian[0] * local_cartesian[0] +
                               local_cartesian[1] * local_cartesian[1]),
                     local_cartesian[2]),
          std::atan2(local_cartesian[1], local_cartesian[0])};
}

}  // namespace bdm

#endif  // CELL_H_
