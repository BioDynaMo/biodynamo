#ifndef CELL_H_
#define CELL_H_

#include <array>
#include <cmath>

#include "daosoa.h"
#include "default_force.h"
#include "inline_vector.h"
#include "math_util.h"
#include "param.h"
#include "simulation_object.h"
#include "simulation_object_util.h"

namespace bdm {

/// \brief This class defines an extension to simulation object that contains
/// fundamental data types and methods.
template <typename Base = SimulationObject<>>
class CellExt : public Base {
  BDM_CLASS_HEADER(CellExt, CellExt<>,
                   CellExt<typename Base::template Self<Backend>>, position_,
                   mass_location_, tractor_force_, diameter_, volume_,
                   adherence_, mass_, neighbors_);

 public:
  CellExt() {}
  explicit CellExt(real_v diameter) : diameter_{diameter} { UpdateVolume(); }
  explicit CellExt(const std::array<real_v, 3>& position)
      : position_(position), mass_location_(position) {}

  virtual ~CellExt() {}

  BDM_FORCE_INLINE const real_v& GetAdherence() const {
    return adherence_[idx_];
  }

  BDM_FORCE_INLINE const real_v& GetDiameter() const { return diameter_[idx_]; }

  BDM_FORCE_INLINE const real_v& GetMass() const { return mass_[idx_]; }

  BDM_FORCE_INLINE const std::array<real_v, 3>& GetMassLocation() const {
    return mass_location_[idx_];
  }

  template <typename T>
  BDM_FORCE_INLINE std::array<aosoa<Self<VcVectorBackend>, VcVectorBackend>,
                              VcVectorBackend::kVecLen>
  GetNeighbors(const T& all_cells) const {
    std::array<aosoa<Self<VcVectorBackend>, VcVectorBackend>,
               VcVectorBackend::kVecLen>
        ret;
    const size_t size = Base::ElementsCurrentVector();
    for (size_t i = 0; i < size; i++) {
      all_cells.Gather(neighbors_[idx_][i], &(ret[i]));
    }
    return ret;
  }

  BDM_FORCE_INLINE const std::array<InlineVector<int, 8>, Backend::kVecLen>
  GetNeighbors() const {
    return neighbors_[idx_];
  }

  BDM_FORCE_INLINE const std::array<real_v, 3>& GetPosition() const {
    return position_[idx_];
  }

  const std::array<real_v, 3>& GetTractorForce() const {
    return tractor_force_[idx_];
  }

  BDM_FORCE_INLINE const real_v& GetVolume() const { return volume_[idx_]; }

  BDM_FORCE_INLINE void SetAdherence(const real_v& adherence) {
    adherence_[idx_] = adherence;
  }

  BDM_FORCE_INLINE void SetDiameter(const real_v& diameter) {
    diameter_[idx_] = diameter;
  }

  BDM_FORCE_INLINE void SetMass(const real_v& mass) { mass_[idx_] = mass; }

  BDM_FORCE_INLINE void SetMassLocation(
      const std::array<real_v, 3>& mass_location) {
    mass_location_[idx_] = mass_location;
  }

  BDM_FORCE_INLINE void SetPosition(const std::array<real_v, 3>& position) {
    position_[idx_] = position;
  }

  BDM_FORCE_INLINE void SetTractorForce(
      const std::array<real_v, 3>& tractor_force) {
    tractor_force_[idx_] = tractor_force;
  }

  BDM_FORCE_INLINE void SetNeighbors(
      const std::array<InlineVector<int, 8>, Backend::kVecLen>& neighbors) {
    neighbors_[idx_] = neighbors;
  }

  BDM_FORCE_INLINE void ChangeVolume(const real_v& speed) {
    // scaling for integration step
    real_v dV = speed * real_t(Param::kSimulationTimeStep);
    volume_[idx_] += dV;
    volume_[idx_] = Vc::iif(volume_[idx_] < real_t(5.2359877E-7),
                            real_v(5.2359877E-7), volume_[idx_]);
    UpdateDiameter();
  }

  BDM_FORCE_INLINE void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    for (size_t i = 0; i < real_v::Size; i++) {  // fixme vectorize that
      diameter_[idx_][i] = std::cbrt(volume_[idx_][i] * 6 / Math::kPi);
    }
  }

  BDM_FORCE_INLINE void UpdateVolume() {
    volume_[idx_] = real_t(Math::kPi) / 6 * diameter_[idx_] * diameter_[idx_] *
                    diameter_[idx_];
  }

  BDM_FORCE_INLINE void UpdateMassLocation(const std::array<real_v, 3>& delta) {
    mass_location_[idx_][0] += delta[0];
    mass_location_[idx_][1] += delta[1];
    mass_location_[idx_][2] += delta[2];
  }

  BDM_FORCE_INLINE void GetForceOn(
      const std::array<real_v, 3>& ref_mass_location,
      const real_v& ref_diameter, std::array<real_v, 3>* force) const {
    DefaultForce<Backend> default_force;  // todo inefficient -> make member
    real_v iof_coefficient(Param::kSphereDefaultInterObjectCoefficient);

    default_force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, mass_location_[idx_],
                                      diameter_[idx_], iof_coefficient, force);
  }

 private:
  BDM_PRIVATE_MEMBER(Container<std::array<real_v COMMA() 3>>, position_);
  BDM_PRIVATE_MEMBER(Container<std::array<real_v COMMA() 3>>, mass_location_);
  BDM_PRIVATE_MEMBER(Container<std::array<real_v COMMA() 3>>, tractor_force_);
  BDM_PRIVATE_MEMBER(Container<real_v>, diameter_);
  BDM_PRIVATE_MEMBER(Container<real_v>, volume_);
  BDM_PRIVATE_MEMBER(Container<real_v>, adherence_);
  BDM_PRIVATE_MEMBER(Container<real_v>, mass_);

  // stores a list of neighbor ids for each scalar cell
  BDM_PRIVATE_MEMBER(Container<SimdArray<InlineVector<int COMMA() 8>>>,
                     neighbors_) = {{}};
};

template <typename Backend = VcVectorBackend>
using Cell = CellExt<SimulationObject<SelectAllMembers, Backend>>;

}  // namespace bdm

#endif  // CELL_H_
