#ifndef DISPLACEMENT_OP_H_
#define DISPLACEMENT_OP_H_

#include <cmath>
#include "backend.h"
#include "cell.h"
#include "math_util.h"
#include "param.h"

namespace bdm {

using real_v = VcBackend::real_v;
using real_t = VcBackend::real_t;
using bool_v = VcBackend::bool_v;

/// Defines the 3D physical interactions between physical objects (cylinders and
/// spheres).
class DisplacementOp {
 public:
  DisplacementOp() {}
  ~DisplacementOp() {}
  DisplacementOp(const DisplacementOp&) = delete;
  DisplacementOp& operator=(const DisplacementOp&) = delete;

  template <typename daosoa>
  void Compute(daosoa* cells) const {
    const size_t n_vectors = cells->vectors();
#pragma omp parallel for
    for (size_t i = 0; i < n_vectors; i++) {
      auto& cell = (*cells)[i];
      // Basically, the idea is to make the sum of all the forces acting
      // on the Point mass. It is stored in translationForceOnPointMass.
      // There is also a computation of the torque (only applied
      // by the daughter neurites), stored in rotationForce.

      // If we detect enough forces to make us  move, we will re-schedule us
      // cell.setOnTheSchedulerListForPhysicalObjects(false);

      const auto& tf = cell.GetTractorForce();

      // the 3 types of movement that can occurq
      // bool_v biological_translation(false);
      bool_v physical_translation(false);
      //      bool_v physical_rotation(false);

      real_v h(Param::kSimulationTimeStep);
      std::array<real_v, 3> movement_at_next_step{
          real_v::Zero(), real_v::Zero(), real_v::Zero()};

      // BIOLOGY :
      // 0) Start with tractor force : What the biology defined as active
      // movement------------
      movement_at_next_step[0] += h * tf[0];
      movement_at_next_step[1] += h * tf[1];
      movement_at_next_step[2] += h * tf[2];

      // PHYSICS
      // the physics force to move the point mass
      std::array<real_v, 3> translation_force_on_point_mass{0, 0, 0};
      // 1) "artificial force" to maintain the sphere in the ecm simulation
      //    boundaries
      // todo add again
      // 2) Spring force from my neurites (translation and rotation)-----------
      // todo add again
      // 3) Object avoidance force --------------------------------------------
      //  (We check for every neighbor object if they touch us, i.e. push us
      //  away)
      const auto& neighbors = cell.GetNeighbors(*cells);
      // todo remove VcBackend with impl.
      for (size_t j = 0; j < VcBackend::kVecLen; j++) {
        for (size_t k = 0; k < neighbors.at(j).vectors(); k++) {
          auto& neighbor = neighbors.at(j)[k];
          std::array<real_v, 3> neighbor_force;

          const auto& cell_mass_location = cell.GetMassLocation();
          std::array<real_v, 3> scalar_mass_location = {
              real_v(cell_mass_location[0][j]),
              real_v(cell_mass_location[1][j]),
              real_v(cell_mass_location[2][j])};
          real_v scalar_diameter(cell.GetDiameter()[j]);
          neighbor.GetForceOn(scalar_mass_location, scalar_diameter,
                              &neighbor_force);
          if (k != neighbors.at(j).vectors() - 1) {
            translation_force_on_point_mass[0][j] += neighbor_force[0].sum();
            translation_force_on_point_mass[1][j] += neighbor_force[1].sum();
            translation_force_on_point_mass[2][j] += neighbor_force[2].sum();
          } else {
            // if vector is not full manually add up forces
            for (size_t l = 0; l < neighbor.Size(); l++) {
              translation_force_on_point_mass[0][j] += neighbor_force[0][l];
              translation_force_on_point_mass[1][j] += neighbor_force[1][l];
              translation_force_on_point_mass[2][j] += neighbor_force[2][l];
            }
          }
        }
      }
      //      for (const auto& neighbor : cell.GetNeighbors()) {
      //        auto force_from_this_neighbor = neighbor->getForceOn(cell);
      //        translation_force_on_point_mass[0] +=
      //        force_from_this_neighbor[0];
      //        translation_force_on_point_mass[1] +=
      //        force_from_this_neighbor[1];
      //        translation_force_on_point_mass[2] +=
      //        force_from_this_neighbor[2];
      //      }
      // 4) PhysicalBonds------------------------------------------------------
      // todo add again

      // How the physics influences the next displacement----------------------
      // todo removed only used in view

      real_v norm_of_force = Vc::sqrt(translation_force_on_point_mass[0] *
                                          translation_force_on_point_mass[0] +
                                      translation_force_on_point_mass[1] *
                                          translation_force_on_point_mass[1] +
                                      translation_force_on_point_mass[2] *
                                          translation_force_on_point_mass[2]);

      // is there enough force to :
      //  - make us biologically move (Tractor) :
      // todo add again biological_translation = norm(tf) > 0.01;
      //  - break adherence and make us translate ?
      physical_translation = norm_of_force > cell.GetAdherence();
      //  - make us rotate ?
      // todo add again

      auto mh = h / cell.GetMass();
      // adding the physics translation (scale by weight) if important enough
      movement_at_next_step[0] +=
          Vc::iif(physical_translation, translation_force_on_point_mass[0] * mh,
                  real_v::Zero());
      movement_at_next_step[1] +=
          Vc::iif(physical_translation, translation_force_on_point_mass[1] * mh,
                  real_v::Zero());
      movement_at_next_step[2] +=
          Vc::iif(physical_translation, translation_force_on_point_mass[2] * mh,
                  real_v::Zero());

      // Performing the translation itself :
      // but we want to avoid huge jumps in the simulation, so there are
      // maximum distances possible
      auto gt_max_displacement =
          norm_of_force * mh > real_t(Param::kSimulationMaximalDisplacement);
      auto max_displacement = Math::Normalize<VcBackend>(movement_at_next_step);
      max_displacement[0] *= real_v(Param::kSimulationMaximalDisplacement);
      max_displacement[1] *= real_v(Param::kSimulationMaximalDisplacement);
      max_displacement[2] *= real_v(Param::kSimulationMaximalDisplacement);

      movement_at_next_step[0] = Vc::iif(
          gt_max_displacement, max_displacement[0], movement_at_next_step[0]);
      movement_at_next_step[1] = Vc::iif(
          gt_max_displacement, max_displacement[1], movement_at_next_step[1]);
      movement_at_next_step[2] = Vc::iif(
          gt_max_displacement, max_displacement[2], movement_at_next_step[2]);

      cell.UpdateMassLocation(movement_at_next_step);
      // FIXME removed rotation
      // updating some values :
      //      UpdateSpatialOrganizationNodePosition(&cell);
      cell.SetPosition(cell.GetMassLocation());
      // Re-schedule me and every one that has something to do with me :
      // cells->setOnTheSchedulerListForPhysicalObjects(true);
      // neighbors :
      //        for (auto neighbor : so_node_->getNeighbors()) {
      //          if (neighbor->isAPhysicalObject()) {
      //            static_cast<PhysicalObject*>(neighbor)->setOnTheSchedulerListForPhysicalObjects(true);
      //          }
      //        }

      // Reset biological movement to 0.
      // (Will need new instruction from SomaElement in order to move again)
      cell.SetTractorForce({real_v::Zero(), real_v::Zero(), real_v::Zero()});
    }
  }

 private:
  void UpdateSpatialOrganizationNodePosition(Cell<VcBackend>* cell) const {
    auto& current_center = cell->GetPosition();
    auto& mass_location = cell->GetMassLocation();
    // fixme can't we pass that as parameter - should be known at call site
    std::array<VcBackend::real_v, 3> displacement = {
        mass_location[0] - current_center[0],
        mass_location[1] - current_center[1],
        mass_location[2] - current_center[2]};
    //    auto offset = Math::Norm<VcBackend>(displacement);
    //    auto& diameter = cell->GetDiameter();
    // todo what is the purpose of this conditional?
    //    auto ifmask = offset > diameter * 0.25 || offset > 5; //fixme magic
    //    numbers 0.25 & 0.0025
    //    auto noise = Random::NextNoise<VcBackend>(diameter * 0.025);
    //    displacement[0] += noise[0];
    //    displacement[1] += noise[1];
    //    displacement[2] += noise[2];
    //    displacement[0].setZeroInverted(ifmask);
    //    displacement[1].setZeroInverted(ifmask);
    //    displacement[2].setZeroInverted(ifmask);
    std::array<VcBackend::real_v, 3> new_position{
        current_center[0] + displacement[0],
        current_center[1] + displacement[1],
        current_center[2] + displacement[2],
    };
    cell->SetPosition(new_position);
  }
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_H_
