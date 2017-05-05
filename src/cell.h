#ifndef CELL_H_
#define CELL_H_

#include <array>
#include <cmath>
#include <type_traits>

#include <Rtypes.h>

#include "backend.h"
#include "cell.h"
#include "default_force.h"
#include "inline_vector.h"
#include "math_util.h"
#include "param.h"
#include "simulation_object.h"
#include "simulation_object_util.h"

namespace bdm {

using std::array;

template <typename Base = SimulationObject<>>
class CellExt : public Base {
  BDM_CLASS_HEADER(CellExt, position_, mass_location_, tractor_force_,
                   diameter_, volume_, adherence_, mass_, neighbors_)

 private:
  static atomic_TClass_ptr fgIsA;

 public:
  static TClass* Class() { throw "foo"; }
  static const char* Class_Name();
  static Version_t Class_Version() { return 1; }
  static TClass* Dictionary();
  virtual TClass* IsA() const { return CellExt::Class(); }
  virtual void ShowMembers(TMemberInspector& insp) const {
    ::ROOT::Class_ShowMembers(CellExt::Class(), this, insp);
  }
  virtual void Streamer(TBuffer&) { throw "foo1"; }

  void StreamerNVirtual(TBuffer& ClassDef_StreamerNVirtual_b) {
    CellExt::Streamer(ClassDef_StreamerNVirtual_b);
  }
  static const char* DeclFileName() { return __FILE__; }
  static int ImplFileLine();
  static const char* ImplFileName();
  static int DeclFileLine() { return __LINE__; }

 public:
  CellExt() {}
  explicit CellExt(TRootIOCtor*) {}  // constructor for ROOT I/O
  explicit CellExt(double diameter) : diameter_(diameter) { UpdateVolume(); }
  explicit CellExt(const array<double, 3>& position)
      : position_(position), mass_location_(position) {}
  virtual ~CellExt() {}

  double GetAdherence() const { return adherence_[idx_]; }

  double GetDiameter() const { return diameter_[idx_]; }

  double GetMass() const { return mass_[idx_]; }

  const array<double, 3>& GetMassLocation() const {
    return mass_location_[idx_];
  }

  const array<double, 3>& GetPosition() const { return position_[idx_]; }

  const array<double, 3>& GetTractorForce() const {
    return tractor_force_[idx_];
  }

  double GetVolume() const { return volume_[idx_]; }

  const InlineVector<int, 8>& GetNeighbors() const { return neighbors_[idx_]; }

  void SetAdherence(double adherence) { adherence_[idx_] = adherence; }

  void SetDiameter(double diameter) { diameter_[idx_] = diameter; }

  void SetMass(double mass) { mass_[idx_] = mass; }

  void SetMassLocation(const array<double, 3>& mass_location) {
    mass_location_[idx_] = mass_location;
  }

  void SetPosition(const array<double, 3>& position) {
    position_[idx_] = position;
  }

  void SetTractorForce(const array<double, 3>& tractor_force) {
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

  void UpdateMassLocation(const array<double, 3>& delta) {
    mass_location_[idx_][0] += delta[0];
    mass_location_[idx_][1] += delta[1];
    mass_location_[idx_][2] += delta[2];
  }

  void GetForceOn(const array<double, 3>& ref_mass_location,
                  double ref_diameter, array<double, 3>* force) const {
    DefaultForce default_force;
    double iof_coefficient = Param::kSphereDefaultInterObjectCoefficient;

    default_force.ForceBetweenSpheres(ref_mass_location, ref_diameter,
                                      iof_coefficient, mass_location_[idx_],
                                      diameter_[idx_], iof_coefficient, force);
  }

 private:
  vec<array<double, 3>> position_;
  vec<array<double, 3>> mass_location_;
  vec<array<double, 3>> tractor_force_;
  vec<double> diameter_;
  vec<double> volume_;
  vec<double> adherence_;
  vec<double> mass_;

  // stores a list of neighbor ids for each scalar cell
  vec<InlineVector<int, 8>> neighbors_;
  // ClassDef(CellExt, 1);  // custom one on top
};

template <typename Backend = Scalar>
using Cell = CellExt<SimulationObject<Backend>>;

}  // namespace bdm

#endif  // CELL_H_
