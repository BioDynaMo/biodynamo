#ifndef NEUROSCIENCE_NEURON_SOMA_H_
#define NEUROSCIENCE_NEURON_SOMA_H_

#include <algorithm>
#include <unordered_map>
#include <vector>
#include "cell.h"
#include "resource_manager.h"
#include "simulation_object_util.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

extern const BmEvent gExtendNeurite;

BDM_SIM_OBJECT(NeuronSoma, bdm::Cell) {
  BDM_SIM_OBJECT_HEADER(NeuronSomaExt, 1, daughters_, daughters_coord_);

 public:
  using NeuriteElement = typename TCompileTimeParam::NeuriteElement;
  using NeuriteElementSoPtr = ToSoPtr<NeuriteElement>;

  NeuronSomaExt() {}

  explicit NeuronSomaExt(const std::array<double, 3>& position)
      : Base(position) {}

  /// Update references of simulation objects that changed its memory position.
  /// @param update_info vector index = type_id, map stores (old_index ->
  /// new_index)
  void UpdateReferences(
      const std::vector<std::unordered_map<uint32_t, uint32_t>>& update_info) {
    // NeuronSoma only stores NeuriteElements
    using Rm = std::remove_pointer_t<decltype(Rm())>;
    const int neurite_type_idx = Rm::template GetTypeIndex<NeuriteElement>();
    const auto& neurite_updates = update_info[neurite_type_idx];
    for (auto& daugther : daughters_[kIdx]) {
      // `this` required, because declaration in dependent base are not found
      // by unqualified look-up.
      this->UpdateReference(&daugther, neurite_updates);
    }
  }

  // ***************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE *
  // ***************************************************************************

  /// Extends a new neurites with default diameter
  /// @param direction direciton of the new neurite
  /// @return SoPointer of new neurite
  NeuriteElementSoPtr ExtendNewNeurite(const std::array<double, 3>& direction) {
    auto dir = Math::Add(direction, Base::position_[kIdx]);
    auto angles = Base::TransformCoordinatesGlobalToPolar(dir);
    return ExtendNewNeurite(Param::kNeuriteDefaultDiameter, angles[2],
                            angles[1]);
  }

  /// Extends a new neurites
  /// @param diameter the diameter of the new neurite
  /// @param phi the angle from the z-axis
  /// @param theta the angle from the x-axis around the z-axis
  /// @return SoPointer of new neurite
  NeuriteElementSoPtr ExtendNewNeurite(double diameter, double phi,
                                       double theta) {
    auto neurite = Rm()->template New<NeuriteElement>();

    std::vector<typename Base::BiologyModules> neurite_bms;
    Base::BiologyModuleEventHandler(gExtendNeurite, &neurite_bms);
    neurite.SetBiologyModules(std::move(neurite_bms));

    double radius = 0.5 * Base::diameter_[kIdx];
    double new_length = Param::kNeuriteDefaultActualLength;
    // position in bdm.cells coord
    double x_coord = std::sin(theta) * std::cos(phi);
    double y_coord = std::sin(theta) * std::sin(phi);
    double z_coord = std::cos(theta);
    std::array<double, 3> axis_direction{
        x_coord * Base::kXAxis[0] + y_coord * Base::kYAxis[0] +
            z_coord * Base::kZAxis[0],
        x_coord * Base::kXAxis[1] + y_coord * Base::kYAxis[1] +
            z_coord * Base::kZAxis[1],
        x_coord * Base::kXAxis[2] + y_coord * Base::kYAxis[2] +
            z_coord * Base::kZAxis[2]};

    // positions & axis in cartesian coord
    auto new_begin_location = Math::Add(
        Base::position_[kIdx], Math::ScalarMult(radius, axis_direction));
    auto new_spring_axis = Math::ScalarMult(new_length, axis_direction);

    auto new_mass_location = Math::Add(new_begin_location, new_spring_axis);

    // set attributes of new neurite segment
    neurite.SetDiameter(diameter);
    neurite.UpdateVolume();
    neurite.SetSpringAxis(new_spring_axis);

    neurite.SetMassLocation(new_mass_location);
    neurite.SetActualLength(new_length);
    neurite.SetRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
    neurite.UpdateLocalCoordinateAxis();

    // family relations
    auto neurite_soptr = neurite.GetSoPtr();
    daughters_[kIdx].push_back(neurite_soptr);
    neurite.SetMother(GetSoPtr());
    daughters_coord_[kIdx][neurite.GetElementIdx()] = {x_coord, y_coord,
                                                       z_coord};

    return neurite_soptr;
  }

  void RemoveDaughter(const ToSoPtr<NeuriteElement> daughter) {
    auto it = std::find(std::begin(daughters_[kIdx]),
                        std::end(daughters_[kIdx]), daughter);
    assert(it != std::end(daughters_[kIdx]) &&
           "The element you wanted to remove is not part of daughters_[kIdx]");
    daughters_[kIdx].erase(it);
  }

  /// Returns the absolute coordinates of the location where the dauther is
  /// attached.
  /// @param daughter_element_idx element_idx of the daughter
  /// @return the coord
  std::array<double, 3> OriginOf(uint32_t daughter_element_idx) const {
    std::array<double, 3> xyz = daughters_coord_[kIdx][daughter_element_idx];

    double radius = Base::diameter_[kIdx] * .5;
    xyz = Math::ScalarMult(radius, xyz);

    const auto& pos = Base::position_[kIdx];

    return {pos[0] + xyz[0] * Base::kXAxis[0] + xyz[1] * Base::kYAxis[0] +
                xyz[2] * Base::kZAxis[0],
            pos[1] + xyz[0] * Base::kXAxis[1] + xyz[1] * Base::kYAxis[1] +
                xyz[2] * Base::kZAxis[1],
            pos[2] + xyz[0] * Base::kXAxis[2] + xyz[1] * Base::kYAxis[2] +
                xyz[2] * Base::kZAxis[2]};
  }

  void UpdateRelative(const ToSoPtr<NeuriteElement>& old_rel,
                      const ToSoPtr<NeuriteElement>& new_rel) {
    auto coord = daughters_coord_[kIdx][old_rel->GetElementIdx()];
    auto it = std::find(std::begin(daughters_[kIdx]),
                        std::end(daughters_[kIdx]), old_rel);
    assert(it != std::end(daughters_[kIdx]) &&
           "old_element_idx could not be found in daughters_ vector");
    *it = new_rel;
    daughters_coord_[kIdx][new_rel->GetElementIdx()] = coord;
  }

  const std::vector<ToSoPtr<NeuriteElement>>& GetDaughters() const {
    return daughters_[kIdx];
  }

 protected:
  vec<std::vector<ToSoPtr<NeuriteElement>>> daughters_ = {{}};

  /// Daughter attachment points in local coordinates
  /// Key: element index of neurite segement
  /// Value: position
  vec<std::unordered_map<uint32_t, std::array<double, 3>>> daughters_coord_ = {
      {}};
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_SOMA_H_
