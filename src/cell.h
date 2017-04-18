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

class SoaCell;
class SoaCellRef;
class SoaCellConstRef;

/// \brief This class defines an extension to simulation object that contains
/// fundamental data types and methods.
class Cell {
 public:
  friend SoaCell;
  friend SoaCellRef;
  friend SoaCellConstRef;

  Cell() {}
  explicit Cell(double diameter) : diameter_{diameter} { UpdateVolume(); }
  explicit Cell(const std::array<double, 3>& position)
      : position_(position), mass_location_(position) {}

  virtual ~Cell() {}

  double GetAdherence() const { return adherence_; }

  double GetDiameter() const { return diameter_; }

  double GetMass() const { return mass_; }

  const std::array<double, 3>& GetMassLocation() const {
    return mass_location_;
  }

  const std::array<double, 3>& GetPosition() const { return position_; }

  const std::array<double, 3>& GetTractorForce() const {
    return tractor_force_;
  }

  double GetVolume() const { return volume_; }

  const InlineVector<int, 8>& GetNeighbors() const { return neighbors_; }

  void SetAdherence(double adherence) { adherence_ = adherence; }

  void SetDiameter(double diameter) { diameter_ = diameter; }

  void SetMass(double mass) { mass_ = mass; }

  void SetMassLocation(const std::array<double, 3>& mass_location) {
    mass_location_ = mass_location;
  }

  void SetPosition(const std::array<double, 3>& position) {
    position_ = position;
  }

  void SetTractorForce(const std::array<double, 3>& tractor_force) {
    tractor_force_ = tractor_force;
  }

  void SetNeighbors(const InlineVector<int, 8>& neighbors) {
    neighbors_ = neighbors;
  }

  void ChangeVolume(double speed) {
    // scaling for integration step
    double dV = speed * Param::kSimulationTimeStep;
    volume_ += dV;
    if (volume_ < 5.2359877E-7) {
      volume_ = 5.2359877E-7;
    }
    UpdateDiameter();
  }

  void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    diameter_ = std::cbrt(volume_ * 6 / Math::kPi);
  }

  void UpdateVolume() {
    volume_ = Math::kPi / 6 * diameter_ * diameter_ * diameter_;
  }

  void UpdateMassLocation(const std::array<double, 3>& delta) {
    mass_location_[0] += delta[0];
    mass_location_[1] += delta[1];
    mass_location_[2] += delta[2];
  }

  void GetForceOn(const std::array<double, 3>& ref_mass_location,
                  double ref_diameter, std::array<double, 3>* force) const {
    DefaultForce default_force;
    double iof_coefficient = Param::kSphereDefaultInterObjectCoefficient;

    default_force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, mass_location_,
                                      diameter_, iof_coefficient, force);
  }

 private:
  std::array<double, 3> position_ = std::array<double, 3>{0, 0, 0};
  std::array<double, 3> mass_location_ = std::array<double, 3>{0, 0, 0};
  std::array<double, 3> tractor_force_ = std::array<double, 3>{0, 0, 0};
  double diameter_;
  double volume_;
  double adherence_;
  double mass_;

  // stores a list of neighbor ids for each scalar cell
  InlineVector<int, 8> neighbors_;
};

class SoaCellConstRef {
 public:
  template <typename T>
  SoaCellConstRef(const T* other, size_t idx);
  virtual ~SoaCellConstRef() {}

  double GetAdherence() const { return adherence_[idx_]; }

  double GetDiameter() const { return diameter_[idx_]; }

  double GetMass() const { return mass_[idx_]; }

  const std::array<double, 3>& GetMassLocation() const {
    return mass_location_[idx_];
  }

  const std::array<double, 3>& GetPosition() const { return position_[idx_]; }

  const std::array<double, 3>& GetTractorForce() const {
    return tractor_force_[idx_];
  }

  double GetVolume() const { return volume_[idx_]; }

  const InlineVector<int, 8>& GetNeighbors() const { return neighbors_[idx_]; }

  // void SetAdherence(double adherence) { adherence_[idx_] = adherence; }
  //
  // void SetDiameter(double diameter) { diameter_[idx_] = diameter; }
  //
  // void SetMass(double mass) { mass_[idx_] = mass; }
  //
  // void SetMassLocation(const std::array<double, 3>& mass_location) {
  //   mass_location_[idx_] = mass_location;
  // }
  //
  // void SetPosition(const std::array<double, 3>& position) {
  //   position_[idx_] = position;
  // }
  //
  // void SetTractorForce(const std::array<double, 3>& tractor_force) {
  //   tractor_force_[idx_] = tractor_force;
  // }
  //
  // void SetNeighbors(const InlineVector<int, 8>& neighbors) {
  //   neighbors_[idx_] = neighbors;
  // }
  //
  // void ChangeVolume(double speed) {
  //   // scaling for integration step
  //   double dV = speed * Param::kSimulationTimeStep;
  //   volume_[idx_] += dV;
  //   if (volume_[idx_] < 5.2359877E-7) {
  //     volume_[idx_] = 5.2359877E-7;
  //   }
  //   UpdateDiameter();
  // }

  // void UpdateDiameter() {
  //   // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
  //   diameter_[idx_] = std::cbrt(volume_[idx_] * 6 / Math::kPi);
  // }
  //
  // void UpdateVolume() {
  //   volume_[idx_] =
  //       Math::kPi / 6 * diameter_[idx_] * diameter_[idx_] * diameter_[idx_];
  // }
  //
  // void UpdateMassLocation(const std::array<double, 3>& delta) {
  //   mass_location_[idx_][0] += delta[0];
  //   mass_location_[idx_][1] += delta[1];
  //   mass_location_[idx_][2] += delta[2];
  // }

  void GetForceOn(const std::array<double, 3>& ref_mass_location,
                  double ref_diameter, std::array<double, 3>* force) const {
    DefaultForce default_force;
    double iof_coefficient = Param::kSphereDefaultInterObjectCoefficient;

    default_force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, mass_location_[idx_],
                                      diameter_[idx_], iof_coefficient, force);
  }

  // void push_back(const Cell& cell) {
  //   position_.push_back(cell.position_);
  //   mass_location_.push_back(cell.mass_location_);
  //   tractor_force_.push_back(cell.tractor_force_);
  //   diameter_.push_back(cell.diameter_);
  //   volume_.push_back(cell.volume_);
  //   adherence_.push_back(cell.adherence_);
  //   mass_.push_back(cell.mass_);
  //   neighbors_.push_back(cell.neighbors_);
  // }
  //
  // void reserve(size_t capacity) {
  //   position_.reserve(capacity);
  //   mass_location_.reserve(capacity);
  //   tractor_force_.reserve(capacity);
  //   diameter_.reserve(capacity);
  //   volume_.reserve(capacity);
  //   adherence_.reserve(capacity);
  //   mass_.reserve(capacity);
  //   neighbors_.reserve(capacity);
  // }

  size_t size() const { return position_.size(); }

  SoaCellConstRef operator[](size_t idx) const {
    return SoaCellConstRef(this, idx);
  }

 private:
  mutable size_t idx_ = 0;
  const std::vector<std::array<double, 3>>& position_;
  const std::vector<std::array<double, 3>>& mass_location_;
  const std::vector<std::array<double, 3>>& tractor_force_;
  const std::vector<double>& diameter_;
  const std::vector<double>& volume_;
  const std::vector<double>& adherence_;
  const std::vector<double>& mass_;

  // stores a list of neighbor ids for each scalar cell
  const std::vector<InlineVector<int, 8>>& neighbors_;
};

template <typename T>
SoaCellConstRef::SoaCellConstRef(const T* other, size_t idx)
      : idx_(idx),
        position_(other->position_),
        mass_location_(other->mass_location_),
        tractor_force_(other->tractor_force_),
        diameter_(other->diameter_),
        volume_(other->volume_),
        adherence_(other->adherence_),
        mass_(other->mass_),
        neighbors_(other->neighbors_) {}

class SoaCellRef {
 public:
  friend SoaCellConstRef;

  template <typename T>
  SoaCellRef(T* other, size_t idx);
  virtual ~SoaCellRef() {}

  double GetAdherence() const { return adherence_[idx_]; }

  double GetDiameter() const { return diameter_[idx_]; }

  double GetMass() const { return mass_[idx_]; }

  const std::array<double, 3>& GetMassLocation() const {
    return mass_location_[idx_];
  }

  const std::array<double, 3>& GetPosition() const { return position_[idx_]; }

  const std::array<double, 3>& GetTractorForce() const {
    return tractor_force_[idx_];
  }

  double GetVolume() const { return volume_[idx_]; }

  const InlineVector<int, 8>& GetNeighbors() const { return neighbors_[idx_]; }

  void SetAdherence(double adherence) { adherence_[idx_] = adherence; }

  void SetDiameter(double diameter) { diameter_[idx_] = diameter; }

  void SetMass(double mass) { mass_[idx_] = mass; }

  void SetMassLocation(const std::array<double, 3>& mass_location) {
    mass_location_[idx_] = mass_location;
  }

  void SetPosition(const std::array<double, 3>& position) {
    position_[idx_] = position;
  }

  void SetTractorForce(const std::array<double, 3>& tractor_force) {
    tractor_force_[idx_] = tractor_force;
  }

  void SetNeighbors(const InlineVector<int, 8>& neighbors) {
    neighbors_[idx_] = neighbors;
  }

  void ChangeVolume(double speed) {
    // scaling for integration step
    double dV = speed * Param::kSimulationTimeStep;
    volume_[idx_] += dV;
    if (volume_[idx_] < 5.2359877E-7) {
      volume_[idx_] = 5.2359877E-7;
    }
    UpdateDiameter();
  }

  void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    diameter_[idx_] = std::cbrt(volume_[idx_] * 6 / Math::kPi);
  }

  void UpdateVolume() {
    volume_[idx_] =
        Math::kPi / 6 * diameter_[idx_] * diameter_[idx_] * diameter_[idx_];
  }

  void UpdateMassLocation(const std::array<double, 3>& delta) {
    mass_location_[idx_][0] += delta[0];
    mass_location_[idx_][1] += delta[1];
    mass_location_[idx_][2] += delta[2];
  }

  void GetForceOn(const std::array<double, 3>& ref_mass_location,
                  double ref_diameter, std::array<double, 3>* force) const {
    DefaultForce default_force;
    double iof_coefficient = Param::kSphereDefaultInterObjectCoefficient;

    default_force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, mass_location_[idx_],
                                      diameter_[idx_], iof_coefficient, force);
  }

  void push_back(const Cell& cell) {
    position_.push_back(cell.position_);
    mass_location_.push_back(cell.mass_location_);
    tractor_force_.push_back(cell.tractor_force_);
    diameter_.push_back(cell.diameter_);
    volume_.push_back(cell.volume_);
    adherence_.push_back(cell.adherence_);
    mass_.push_back(cell.mass_);
    neighbors_.push_back(cell.neighbors_);
  }

  void reserve(size_t capacity) {
    position_.reserve(capacity);
    mass_location_.reserve(capacity);
    tractor_force_.reserve(capacity);
    diameter_.reserve(capacity);
    volume_.reserve(capacity);
    adherence_.reserve(capacity);
    mass_.reserve(capacity);
    neighbors_.reserve(capacity);
  }

  size_t size() const { return position_.size(); }

  SoaCellRef operator[](size_t idx) {
    return SoaCellRef(this, idx);
  }

  SoaCellConstRef operator[](size_t idx) const {
    return SoaCellConstRef(this, idx);
  }

 private:
  mutable size_t idx_ = 0;
  std::vector<std::array<double, 3>>& position_;
  std::vector<std::array<double, 3>>& mass_location_;
  std::vector<std::array<double, 3>>& tractor_force_;
  std::vector<double>& diameter_;
  std::vector<double>& volume_;
  std::vector<double>& adherence_;
  std::vector<double>& mass_;

  // stores a list of neighbor ids for each scalar cell
  std::vector<InlineVector<int, 8>>& neighbors_;
};

class SoaCell {
 public:
  friend SoaCellRef;
  friend SoaCellConstRef;

  SoaCell() {}
  explicit SoaCell(double capacity) { reserve(capacity); }
  virtual ~SoaCell() {}

  double GetAdherence() const { return adherence_[idx_]; }

  double GetDiameter() const { return diameter_[idx_]; }

  double GetMass() const { return mass_[idx_]; }

  const std::array<double, 3>& GetMassLocation() const {
    return mass_location_[idx_];
  }

  const std::array<double, 3>& GetPosition() const { return position_[idx_]; }

  const std::array<double, 3>& GetTractorForce() const {
    return tractor_force_[idx_];
  }

  double GetVolume() const { return volume_[idx_]; }

  const InlineVector<int, 8>& GetNeighbors() const { return neighbors_[idx_]; }

  void SetAdherence(double adherence) { adherence_[idx_] = adherence; }

  void SetDiameter(double diameter) { diameter_[idx_] = diameter; }

  void SetMass(double mass) { mass_[idx_] = mass; }

  void SetMassLocation(const std::array<double, 3>& mass_location) {
    mass_location_[idx_] = mass_location;
  }

  void SetPosition(const std::array<double, 3>& position) {
    position_[idx_] = position;
  }

  void SetTractorForce(const std::array<double, 3>& tractor_force) {
    tractor_force_[idx_] = tractor_force;
  }

  void SetNeighbors(const InlineVector<int, 8>& neighbors) {
    neighbors_[idx_] = neighbors;
  }

  void ChangeVolume(double speed) {
    // scaling for integration step
    double dV = speed * Param::kSimulationTimeStep;
    volume_[idx_] += dV;
    if (volume_[idx_] < 5.2359877E-7) {
      volume_[idx_] = 5.2359877E-7;
    }
    UpdateDiameter();
  }

  void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    diameter_[idx_] = std::cbrt(volume_[idx_] * 6 / Math::kPi);
  }

  void UpdateVolume() {
    volume_[idx_] =
        Math::kPi / 6 * diameter_[idx_] * diameter_[idx_] * diameter_[idx_];
  }

  void UpdateMassLocation(const std::array<double, 3>& delta) {
    mass_location_[idx_][0] += delta[0];
    mass_location_[idx_][1] += delta[1];
    mass_location_[idx_][2] += delta[2];
  }

  void GetForceOn(const std::array<double, 3>& ref_mass_location,
                  double ref_diameter, std::array<double, 3>* force) const {
    DefaultForce default_force;
    double iof_coefficient = Param::kSphereDefaultInterObjectCoefficient;

    default_force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, mass_location_[idx_],
                                      diameter_[idx_], iof_coefficient, force);
  }

  void push_back(const Cell& cell) {
    position_.push_back(cell.position_);
    mass_location_.push_back(cell.mass_location_);
    tractor_force_.push_back(cell.tractor_force_);
    diameter_.push_back(cell.diameter_);
    volume_.push_back(cell.volume_);
    adherence_.push_back(cell.adherence_);
    mass_.push_back(cell.mass_);
    neighbors_.push_back(cell.neighbors_);
  }

  void reserve(size_t capacity) {
    position_.reserve(capacity);
    mass_location_.reserve(capacity);
    tractor_force_.reserve(capacity);
    diameter_.reserve(capacity);
    volume_.reserve(capacity);
    adherence_.reserve(capacity);
    mass_.reserve(capacity);
    neighbors_.reserve(capacity);
  }

  size_t size() const { return position_.size(); }

  SoaCellRef operator[](size_t idx) {
    return SoaCellRef(this, idx);
  }

  SoaCellConstRef operator[](size_t idx) const {
    return SoaCellConstRef(this, idx);
  }

 private:
  mutable size_t idx_ = 0;
  std::vector<std::array<double, 3>> position_;
  std::vector<std::array<double, 3>> mass_location_;
  std::vector<std::array<double, 3>> tractor_force_;
  std::vector<double> diameter_;
  std::vector<double> volume_;
  std::vector<double> adherence_;
  std::vector<double> mass_;

  // stores a list of neighbor ids for each scalar cell
  std::vector<InlineVector<int, 8>> neighbors_;
};

template <typename T>
SoaCellRef::SoaCellRef(T* other, size_t idx)
      : idx_(idx),
        position_(other->position_),
        mass_location_(other->mass_location_),
        tractor_force_(other->tractor_force_),
        diameter_(other->diameter_),
        volume_(other->volume_),
        adherence_(other->adherence_),
        mass_(other->mass_),
        neighbors_(other->neighbors_) {}

}  // namespace bdm

#endif  // CELL_H_
