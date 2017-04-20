#ifndef DISPLACEMENT_OP_H_
#define DISPLACEMENT_OP_H_

#include <array>
#include <cmath>
#include "math_util.h"
#include "param.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects (cylinders and
/// spheres).
class DisplacementOp {
 public:
  DisplacementOp() {}
  ~DisplacementOp() {}
  DisplacementOp(const DisplacementOp&) = delete;
  DisplacementOp& operator=(const DisplacementOp&) = delete;

  template <typename TContainer>
  void Compute(TContainer* cells) const {
#pragma omp parallel for
      for (size_t i = 0; i < cells->size(); i++) {
        auto&& cell = (*cells)[i];
        // Basically, the idea is to make the sum of all the forces acting
        // on the Point mass. It is stored in translationForceOnPointMass.
        // There is also a computation of the torque (only applied
        // by the daughter neurites), stored in rotationForce.

        // TODO : There might be a problem, in the sense that the biology is not
        // applied
        // if the total Force is smaller than adherence. Once, I should look at
        // this more carefully.

        // If we detect enough forces to make us  move, we will re-schedule us
        // setOnTheSchedulerListForPhysicalObjects(false);

        // fixme why? copying
        const auto& tf = cell.GetTractorForce();

        // the 3 types of movement that can occur
        // bool biological_translation = false;
        bool physical_translation = false;
        // bool physical_rotation = false;

        double h = Param::kSimulationTimeStep;
        std::array<double, 3> movement_at_next_step{0, 0, 0};

        // BIOLOGY :
        // 0) Start with tractor force : What the biology defined as active
        // movement------------
        movement_at_next_step[0] += h * tf[0];
        movement_at_next_step[1] += h * tf[1];
        movement_at_next_step[2] += h * tf[2];

        // PHYSICS
        // the physics force to move the point mass
        std::array<double, 3> translation_force_on_point_mass{0, 0, 0};
        // the physics force to rotate the cell
        // std::array<double, 3> rotation_force { 0, 0, 0 };

        // 1) "artificial force" to maintain the sphere in the ecm simulation
        // boundaries--------
        // 2) Spring force from my neurites (translation and
        // rotation)--------------------------
        // 3) Object avoidance force
        // -----------------------------------------------------------
        //  (We check for every neighbor object if they touch us, i.e. push us
        //  away)
        const auto& neighbor_ids = cell.GetNeighbors();
        for (size_t j = 0; j < neighbor_ids.size(); j++) {
          const auto& neighbor = (*cells)[neighbor_ids[j]];
          std::array<double, 3> neighbor_force;
          // cell.GetForceOn(
          // (*thread_safe_cells)[neighbor_ids[j]].GetMassLocation(),
          // (*thread_safe_cells)[neighbor_ids[j]].GetDiameter(),
          // &neighbor_force);
          neighbor.GetForceOn(
              cell.GetMassLocation(),
              cell.GetDiameter(), &neighbor_force);
          translation_force_on_point_mass[0] += neighbor_force[0];
          translation_force_on_point_mass[1] += neighbor_force[1];
          translation_force_on_point_mass[2] += neighbor_force[2];
          // }
        }

        // 4)
        // PhysicalBonds--------------------------------------------------------------------
        // How the physics influences the next
        // displacement--------------------------------------------------------
        double norm_of_force =
            std::sqrt(translation_force_on_point_mass[0] *
                          translation_force_on_point_mass[0] +
                      translation_force_on_point_mass[1] *
                          translation_force_on_point_mass[1] +
                      translation_force_on_point_mass[2] *
                          translation_force_on_point_mass[2]);

        // is there enough force to :
        //  - make us biologically move (Tractor) :
        //  - break adherence and make us translate ?
        physical_translation =
            norm_of_force > cell.GetAdherence();

        double mh = h / cell.GetMass();
        // adding the physics translation (scale by weight) if important enough
        if (physical_translation) {
          // We scale the move with mass and time step
          movement_at_next_step[0] += translation_force_on_point_mass[0] * mh;
          movement_at_next_step[1] += translation_force_on_point_mass[1] * mh;
          movement_at_next_step[2] += translation_force_on_point_mass[2] * mh;

          // Performing the translation itself :

          // but we want to avoid huge jumps in the simulation, so there are
          // maximum distances possible
          if (norm_of_force * mh > Param::kSimulationMaximalDisplacement) {
            // movement_at_next_step =
            // Matrix::scalarMult(Param::kSimulationMaximalDisplacement,
            //                                            Matrix::normalize(movement_at_next_step));
            const auto& norm = Math::Normalize(movement_at_next_step);
            movement_at_next_step[0] =
                norm[0] * Param::kSimulationMaximalDisplacement;
            movement_at_next_step[1] =
                norm[1] * Param::kSimulationMaximalDisplacement;
            movement_at_next_step[2] =
                norm[2] * Param::kSimulationMaximalDisplacement;
          }

          // // The translational movement itself
          // mass_location_[0] += movement_at_next_step[0];
          // mass_location_[1] += movement_at_next_step[1];
          // mass_location_[2] += movement_at_next_step[2];
        }
        // Performing the rotation
        // updating some values :
        cell.UpdateMassLocation(movement_at_next_step);
        cell.SetPosition(
            cell.GetMassLocation());

        // Reset biological movement to 0.
        // (Will need new instruction from SomaElement in order to move again)
        cell.SetTractorForce({0, 0, 0});
      }
  }
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_H_
