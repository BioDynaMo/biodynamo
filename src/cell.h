#ifndef CELL_H_
#define CELL_H_

#include <array>
#include <cmath>
#include "backend.h"
#include "daosoa.h"
#include "default_force.h"
#include "inline_vector.h"
#include "math_util.h"
#include "param.h"

namespace bdm {

template <typename Backend>
class Cell {
 public:
  using real_v = typename Backend::real_v;
  using real_t = typename Backend::real_t;
  using bool_v = typename Backend::bool_v;

  template <typename>
  friend class Cell;

  Cell() {}
  explicit Cell(real_v diameter) : diameter_{diameter} { UpdateVolume(); }
  explicit Cell(const std::array<real_v, 3>& position)
      : position_(position), mass_location_(position) {}

  virtual ~Cell() {}

  // code related to SOA
  Vc_ALWAYS_INLINE void push_back(const Cell<ScalarBackend>& cell);

  Vc_ALWAYS_INLINE bool is_full() const { return size_ == Backend::kVecLen; }

  Vc_ALWAYS_INLINE size_t Size() const { return size_; }

  // Vc_ALWAYS_INLINE void SetUninitialized() { size_ = 0; }

  Vc_ALWAYS_INLINE void SetSize(std::size_t size) { size_ = size; }

  Vc_ALWAYS_INLINE void Set(std::size_t index, const Cell<ScalarBackend>& cell);

  Vc_ALWAYS_INLINE Cell<ScalarBackend> Get(std::size_t index) const;

  Vc_ALWAYS_INLINE void CopyTo(std::size_t from_idx, std::size_t to_idx,
                               Cell<VcBackend>* dest) const;

  // remaining functions
  Vc_ALWAYS_INLINE const real_v& GetAdherence() const { return adherence_; }

  Vc_ALWAYS_INLINE const real_v& GetDiameter() const { return diameter_; }

  Vc_ALWAYS_INLINE const real_v& GetMass() const { return mass_; }

  Vc_ALWAYS_INLINE const std::array<real_v, 3>& GetMassLocation() const {
    return mass_location_;
  }

  Vc_ALWAYS_INLINE std::array<aosoa<::bdm::Cell<Backend>, Backend>, Backend::kVecLen>
  GetNeighbors(const daosoa<::bdm::Cell, Backend>& all_cells) const {
    std::array<aosoa<::bdm::Cell<Backend>, Backend>, Backend::kVecLen> ret;
    const size_t size = size_;
    for (size_t i = 0; i < size; i++) {
      all_cells.Gather(neighbors_[i], &(ret[i]));
    }
    return ret;
  }

  Vc_ALWAYS_INLINE const std::array<InlineVector<int, 8>, Backend::kVecLen>
  GetNeighbors() const {
    return neighbors_;
  }

  Vc_ALWAYS_INLINE const std::array<real_v, 3>& GetPosition() const {
    return position_;
  }

  Vc_ALWAYS_INLINE const std::array<real_v, 3>& GetTractorForce() const {
    return tractor_force_;
  }

  Vc_ALWAYS_INLINE const real_v& GetVolume() const { return volume_; }

  Vc_ALWAYS_INLINE void SetAdherence(const real_v& adherence) {
    adherence_ = adherence;
  }
  Vc_ALWAYS_INLINE void SetDiameter(const real_v& diameter) {
    diameter_ = diameter;
  }
  Vc_ALWAYS_INLINE void SetMass(const real_v& mass) { mass_ = mass; }
  Vc_ALWAYS_INLINE void SetMassLocation(
      const std::array<real_v, 3>& mass_location) {
    mass_location_ = mass_location;
  }
  Vc_ALWAYS_INLINE void SetPosition(const std::array<real_v, 3>& position) {
    position_ = position;
  }
  Vc_ALWAYS_INLINE void SetTractorForce(
      const std::array<real_v, 3>& tractor_force) {
    tractor_force_ = tractor_force;
  }
  Vc_ALWAYS_INLINE void SetNeighbors(
      const std::array<InlineVector<int, 8>, Backend::kVecLen>& neighbors) {
    neighbors_ = neighbors;
  }

  // todo move all decleration down to declaration section
  Vc_ALWAYS_INLINE void ChangeVolume(const real_v& speed) {
    // scaling for integration step
    real_v dV = speed * real_t(Param::kSimulationTimeStep);
    volume_ += dV;
    volume_ =
        Vc::iif(volume_ < real_t(5.2359877E-7), real_v(5.2359877E-7), volume_);
    UpdateDiameter();
    //    scheduleMeAndAllMyFriends();
  }

  Vc_ALWAYS_INLINE void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    for (size_t i = 0; i < real_v::Size; i++) {  // fixme vectorize that
      diameter_[i] = std::cbrt(volume_[i] * 6 / Math::kPi);
    }
  }

  Vc_ALWAYS_INLINE void UpdateVolume() {
    volume_ = real_t(Math::kPi) / 6 * diameter_ * diameter_ * diameter_;
  }

  Vc_ALWAYS_INLINE void UpdateMassLocation(const std::array<real_v, 3>& delta) {
    mass_location_[0] += delta[0];
    mass_location_[1] += delta[1];
    mass_location_[2] += delta[2];
  }

  Vc_ALWAYS_INLINE void GetForceOn(
      const std::array<real_v, 3>& ref_mass_location,
      const real_v& ref_diameter, std::array<real_v, 3>* force) const {
    DefaultForce<Backend> default_force;  // todo inefficient -> make member
    real_v iof_coefficient(Param::kSphereDefaultInterObjectCoefficient);

    default_force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, mass_location_,
                                      diameter_, iof_coefficient, force);
  }

 private:
  std::size_t size_ = Backend::kVecLen;

  std::array<real_v, 3> position_;
  std::array<real_v, 3> mass_location_;
  real_v diameter_;
  real_v volume_;
  std::array<real_v, 3> tractor_force_;
  real_v adherence_;
  real_v mass_;
  // stores a list of neighbor ids for each scalar cell
  std::array<InlineVector<int, 8>, Backend::kVecLen> neighbors_;
};

// ----------------------------------------------------------------------------
// declarations
// ----------------------------------------------------------------------------

template <typename Backend>
void Cell<Backend>::push_back(const Cell<ScalarBackend>& cell) {
  Set(size_++, cell);
}

// todo get for vector backend

// fixme enable only if Backend != ScalarBackend
template <typename Backend>
inline Cell<ScalarBackend> Cell<Backend>::Get(std::size_t index) const {
  // todo error if index out of bounds
  Cell<ScalarBackend> cell;
  // copy all members;
  real_t pos_x = position_[0][index];
  real_t pos_y = position_[1][index];
  real_t pos_z = position_[2][index];
  cell.position_ = {pos_x, pos_y, pos_z};
  real_t mass_location_x = mass_location_[0][index];
  real_t mass_location_y = mass_location_[1][index];
  real_t mass_location_z = mass_location_[2][index];
  cell.mass_location_ = {mass_location_x, mass_location_y, mass_location_z};
  cell.diameter_ = {diameter_[index]};
  cell.volume_ = {volume_[index]};
  real_t tf_x = tractor_force_[0][index];
  real_t tf_y = tractor_force_[1][index];
  real_t tf_z = tractor_force_[2][index];
  cell.tractor_force_ = {tf_x, tf_y, tf_z};
  cell.adherence_ = {adherence_[index]};
  cell.mass_ = {mass_[index]};
  return cell;
}

template <typename Backend>
void Cell<Backend>::Set(std::size_t index, const Cell<ScalarBackend>& cell) {
  // todo better error message
  throw std::runtime_error("should never be called");
}

template <>
inline void Cell<VcBackend>::Set(std::size_t index,
                                 const Cell<ScalarBackend>& cell) {
  // todo error if index out of bounds
  position_[0][index] = cell.position_[0][0];
  position_[1][index] = cell.position_[1][0];
  position_[2][index] = cell.position_[2][0];
  mass_location_[0][index] = cell.mass_location_[0][0];
  mass_location_[1][index] = cell.mass_location_[1][0];
  mass_location_[2][index] = cell.mass_location_[2][0];
  diameter_[index] = cell.diameter_[0];
  volume_[index] = cell.volume_[0];
  tractor_force_[0][index] = cell.tractor_force_[0][0];
  tractor_force_[1][index] = cell.tractor_force_[1][0];
  tractor_force_[2][index] = cell.tractor_force_[2][0];
  adherence_[index] = cell.adherence_[0];
  mass_[index] = cell.mass_[0];
}

// todo only for vector backend
template <typename Backend>
inline void Cell<Backend>::CopyTo(std::size_t src_idx, std::size_t dest_idx,
                                  Cell<VcBackend>* dest) const {
  dest->position_[0][dest_idx] = position_[0][src_idx];
  dest->position_[1][dest_idx] = position_[1][src_idx];
  dest->position_[2][dest_idx] = position_[2][src_idx];
  dest->mass_location_[0][dest_idx] = mass_location_[0][src_idx];
  dest->mass_location_[1][dest_idx] = mass_location_[1][src_idx];
  dest->mass_location_[2][dest_idx] = mass_location_[2][src_idx];
  dest->diameter_[dest_idx] = diameter_[src_idx];
  dest->volume_[dest_idx] = volume_[src_idx];
  dest->tractor_force_[0][dest_idx] = tractor_force_[0][src_idx];
  dest->tractor_force_[1][dest_idx] = tractor_force_[1][src_idx];
  dest->tractor_force_[2][dest_idx] = tractor_force_[2][src_idx];
  dest->adherence_[dest_idx] = adherence_[src_idx];
  dest->mass_[dest_idx] = mass_[src_idx];
}

}  // namespace bdm

#endif  // CELL_H_
