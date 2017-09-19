#ifndef DISPLACEMENT_OP_H_
#define DISPLACEMENT_OP_H_

#include <array>
#include <cmath>
#include <vector>
#include "grid.h"
#include "math_util.h"
#include "param.h"
#include "simulation_object_vector.h"

namespace bdm {

using std::vector;
using std::array;

/// Defines the 3D physical interactions between physical objects
template <typename TGrid = Grid<>,
          typename TResourceManager = ResourceManager<>>
class DisplacementOp {
 public:
  DisplacementOp() {}
  ~DisplacementOp() {}

  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) const {
    vector<array<double, 3>> cell_movements;
    cell_movements.reserve(cells->size());

    auto& grid = TGrid::GetInstance();
    auto search_radius = grid.GetLargestObjectSize();
    double squared_radius = search_radius * search_radius;

    force_.Initialize();

    auto calculate_neighbor_forces = [&](auto&& sim_object1, SoHandle handle1,
                                         auto&& sim_object2, SoHandle handle2) {
      std::array<double, 3> neighbor_force;
      sim_object2.GetForceOn(sim_object1.GetMassLocation(),
                             sim_object1.GetDiameter(), &neighbor_force);
      if (neighbor_force[0] != 0.0 || neighbor_force[1] != 0.0 ||
          neighbor_force[2] != 0.0) {
        force_[handle1][0] += neighbor_force[0];
        force_[handle1][1] += neighbor_force[1];
        force_[handle1][2] += neighbor_force[2];

        force_[handle2][0] -= neighbor_force[0];
        force_[handle2][1] -= neighbor_force[1];
        force_[handle2][2] -= neighbor_force[2];
      }
    };

    grid.ForEachNeighborPairWithinRadius(calculate_neighbor_forces,
                                         squared_radius);

#pragma omp parallel for shared(grid) firstprivate(squared_radius)
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      // Basically, the idea is to make the sum of all the forces acting
      // on the Point mass. It is stored in translationForceOnPointMass.
      // There is also a computation of the torque (only applied
      // by the daughter neurites), stored in rotationForce.

      // TODO(roman) : There might be a problem, in the sense that the biology
      // is not applied if the total Force is smaller than adherence.
      // Once, I should look at this more carefully.

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
      const std::array<double, 3>& translation_force_on_point_mass =
          force_[SoHandle(type_idx, i)];
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

      // 4) PhysicalBonds
      // How the physics influences the next displacement
      double norm_of_force = std::sqrt(translation_force_on_point_mass[0] *
                                           translation_force_on_point_mass[0] +
                                       translation_force_on_point_mass[1] *
                                           translation_force_on_point_mass[1] +
                                       translation_force_on_point_mass[2] *
                                           translation_force_on_point_mass[2]);

      // is there enough force to :
      //  - make us biologically move (Tractor) :
      //  - break adherence and make us translate ?
      physical_translation = norm_of_force > cell.GetAdherence();

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
          const auto& norm = Math::Normalize(movement_at_next_step);
          movement_at_next_step[0] =
              norm[0] * Param::kSimulationMaximalDisplacement;
          movement_at_next_step[1] =
              norm[1] * Param::kSimulationMaximalDisplacement;
          movement_at_next_step[2] =
              norm[2] * Param::kSimulationMaximalDisplacement;
        }
      }
      cell_movements[i] = movement_at_next_step;
    }

// set new positions after all updates have been calculated
// otherwise some cells would see neighbors with already updated positions
// which would lead to inconsistencies
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      cell.UpdateMassLocation(cell_movements[i]);
      cell.SetPosition(cell.GetMassLocation());

      // Reset biological movement to 0.
      cell.SetTractorForce({0, 0, 0});
    }
  }

 private:
  /// stores force from neighbors for each simulation object
  mutable SimulationObjectVector<std::array<double, 3>, TResourceManager> force_;
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_H_
