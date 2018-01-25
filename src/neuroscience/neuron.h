#ifndef NEUROSCIENCE_NEURON_H_
#define NEUROSCIENCE_NEURON_H_

#include <unordered_map>
#include <typeinfo>  // TODO remove
#include "cell.h"
#include "simulation_object_util.h"
#include "resource_manager.h"

namespace bdm {
namespace neuroscience {

BDM_SIM_OBJECT(Neuron, Cell) {
  BDM_SIM_OBJECT_HEADER(NeuronExt, 1, daughters_, daughters_coord_, foo_);

 public:
  using TNeurite = typename TCompileTimeParam::TNeurite;
  using TNeuriteSoPtr = ToSoPtr<TNeurite>;

  NeuronExt() {
    // TODO remove
    // std::cout << typeid(MostDerived).name() << std::endl;
    // std::cout << "   Neurite Soa " << typeid(typename TNeurite::template Self<Soa>).name() << std::endl;
    // std::cout << "   Neurite Sca " << typeid(typename TNeurite::template Self<Scalar>).name() << std::endl;
  }

  NeuronExt(const std::array<double, 3>& position) : Base(position) {}

  // *************************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE                                            *
  // *************************************************************************************

  TNeuriteSoPtr ExtendNewNeurite(const std::array<double, 3>& direction);

  TNeuriteSoPtr ExtendNewNeurite(double diameter, double phi, double theta);

  void RemoveDaughter(const ToSoPtr<TNeurite> daughter);

  /// Returns the absolute coordinates of the location where the dauther is
  /// attached.
  /// @param daughter_element_idx element_idx of the daughter
  /// @return the coord
  std::array<double, 3> OriginOf(uint32_t daughter_element_idx) const;

 private:
  // vec<SoPointer<typename ToBackend<TNeurite, SimBackend>::type, SimBackend>>
  // daughters_;
  // vec<SoPointer<ToBackend<TNeurite, SimBackend>, SimBackend>> daughters_;
  vec<std::vector<ToSoPtr<TNeurite>>> daughters_ = {{}};

  /// Daughter attachment points in local coordinates
  /// Key: element index of neurite segement
  /// Value: position
  /// FIXME update if elemement index changes.
  vec<std::unordered_map<uint32_t, std::array<double, 3>>> daughters_coord_ = {{}};

  // TNeurite* bar_;

  // vec<SoPointer<typename TNeurite::template Self<SimBackend>, SimBackend>> aa_;

  // vec<SoPointer<TNeuron<SimBackend>, SimBackend>> foo_;
  vec<MostDerivedSoPtr> foo_;
  // vec<MostDerived*> foo_;
  // using TNeuron instead of Self<Backend> in case a customized neuron is used
  //  vec<SoPointer<TNeuron, SimBackend>> bar_;
  // TNeuron* bar_;
};

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
BDM_SO_DEFINE(inline void NeuronExt)::RemoveDaughter(const ToSoPtr<typename TCompileTimeParam::TNeurite> daugther) {

}

BDM_SO_DEFINE(inline typename NeuronExt<TCompileTimeParam, TDerived, TBase>::TNeuriteSoPtr NeuronExt)::ExtendNewNeurite(const std::array<double, 3>& direction) {
  auto dir = Matrix::Add(direction, Base::position_[kIdx]);
  auto angles = Base::TransformCoordinatesGlobalToPolar(dir);
  return ExtendNewNeurite(Param::kNeuriteDefaultDiameter, angles[1], angles[2]);
}

BDM_SO_DEFINE(inline typename NeuronExt<TCompileTimeParam, TDerived, TBase>::TNeuriteSoPtr NeuronExt)::ExtendNewNeurite(double diameter, double phi, double theta) {
  // TODO should this take immediate effect? or delayed + commit?
  auto neurite = Rm()->template New<TNeurite>();
  // TODO copy biological modules
  // for (auto module : local_biology_modules_) {
  //   if (module->isCopiedWhenNeuriteExtendsFromSoma())
  //     ne->addLocalBiologyModule(module->getCopy());
  // }

  // TODO remove comment: code from PhysicalSphere::addNewPhysicalCylinder
  double radius = 0.5 * Base::diameter_[kIdx];
  double new_length = Param::kNeuriteDefaultActualLength;
  // position in bdm.cells coord
  double x_coord = std::cos(theta) * std::sin(phi);
  double y_coord = std::sin(theta) * std::sin(phi);
  double z_coord = std::cos(phi);
  std::array<double, 3> axis_direction { x_coord * Base::kXAxis[0] + y_coord * Base::kYAxis[0] + z_coord * Base::kZAxis[0], x_coord
      * Base::kXAxis[1] + y_coord * Base::kYAxis[1] + z_coord * Base::kZAxis[1], x_coord * Base::kXAxis[2] + y_coord * Base::kYAxis[2]
      + z_coord * Base::kZAxis[2] };

  // positions & axis in cartesian coord
  // TODO rename variables
  auto new_cyl_begin_location = Matrix::Add(Base::position_[kIdx], Matrix::ScalarMult(radius, axis_direction));
  auto new_cyl_spring_axis = Matrix::ScalarMult(new_length, axis_direction);

  auto new_position = Matrix::Add(new_cyl_begin_location, new_cyl_spring_axis);
  // TODO remove
  // auto new_cyl_central_node_location = Matrix::Add(new_cyl_begin_location,
  //                                                  Matrix::ScalarMult(0.5, new_cyl_spring_axis));
  // set attributes of new neurite segment
  neurite.SetDiameter(diameter);
  neurite.UpdateVolume();
  neurite.SetSpringAxis(new_cyl_spring_axis);
  auto& foo = neurite.GetSpringAxis();

  neurite.SetMassLocation(new_position); // TODO rename variable
  neurite.SetActualLength(new_length);
  neurite.SetRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
  neurite.UpdateLocalCoordinateAxis();

  // family relations
  auto neurite_soptr = neurite.GetSoPtr();
  daughters_[kIdx].push_back(neurite_soptr);
  neurite.SetMother(GetSoPtr());
  daughters_coord_[kIdx][neurite.GetElementIdx()] = {x_coord, y_coord, z_coord};

  return neurite_soptr;
}

BDM_SO_DEFINE(inline std::array<double, 3> NeuronExt)::OriginOf(uint32_t daughter_element_idx) const {
  std::array<double, 3> xyz = daughters_coord_[kIdx][daughter_element_idx];

  double radius = Base::diameter_[kIdx] * .5;
  xyz = Matrix::ScalarMult(radius, xyz);

  const auto& pos = Base::position_[kIdx];


  return {
    pos[0] + xyz[0] * Base::kXAxis[0] + xyz[1] * Base::kYAxis[0] + xyz[2] * Base::kZAxis[0],
    pos[1] + xyz[0] * Base::kXAxis[1] + xyz[1] * Base::kYAxis[1] + xyz[2] * Base::kZAxis[1],
    pos[2] + xyz[0] * Base::kXAxis[2] + xyz[1] * Base::kYAxis[2] + xyz[2] * Base::kZAxis[2]
  };
}


BDM_SIM_OBJECT(SpecializedNeuron, Neuron) {
  BDM_SIM_OBJECT_HEADER(SpecializedNeuronExt, 1, me_);
 public:
  SpecializedNeuronExt() {
    // auto&& me = me_[kIdx].Get();
  }

  SpecializedNeuronExt(const std::array<double, 3>& position) : Base(position) {}

 private:
  vec<SoPointer<MostDerivedSB, SimBackend>> me_;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_H_
