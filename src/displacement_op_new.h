#ifndef DISPLACEMENT_OP_NEW_H_
#define DISPLACEMENT_OP_NEW_H_

#include <array>
#include <cmath>
#include <vector>
#include <thread>
#include <unordered_map>
#include <omp.h>
#include <atomic>
#include "grid.h"
#include "math_util.h"
#include "param.h"
#include "omp.h"

namespace bdm {

using std::vector;
using std::array;

/// Defines the 3D physical interactions between physical objects
class DisplacementOpNew {
 public:
  DisplacementOpNew() {
    // force.resize(128*128*128);
  }
  ~DisplacementOpNew() {}
  DisplacementOpNew(const DisplacementOpNew&) = delete;
  DisplacementOpNew& operator=(const DisplacementOpNew&) = delete;

  void AtomicDoubleAdd(std::atomic<double>& a, double value) {
    auto current = a.load();
    while (!a.compare_exchange_weak(current, current + value))
      ;
  }

  void OmpAtomicAdd(double& old, double add) {
    #pragma omp atomic
    old += add;
  }

// std::vector<std::array<std::atomic<double>, 3>> force;

  template <typename TContainer>
  void Compute(TContainer* cells) {
    vector<array<double, 3>> cell_movements;
    cell_movements.reserve(cells->size());

    auto& grid = Grid::GetInstance();
    auto search_radius = grid.GetLargestObjectSize();
    double squared_radius = search_radius * search_radius;

    // force.resize(cells->size());
    const uint64_t num_threads = omp_get_max_threads();
    auto* force = new std::array<double, 3>[cells->size() * num_threads];
    #pragma omp parallel for
    for(uint64_t i = 0; i < cells->size() * num_threads; i++) {
      force[i][0] = 0;
      force[i][1] = 0;
      force[i][2] = 0;
    }

    // auto total_num_threads = omp_get_num_threads();
    // // std::vector<std::unordered_map<size_t, std::array<double, 3>>> thread_local_force;
    // std::vector<decltype(force)> thread_local_force;
    // thread_local_force.resize(total_num_threads);
    // auto expected_neighbors_per_thread = cells->size();// / total_num_threads;
    // for(auto& element : thread_local_force) {
    //   element.resize(expected_neighbors_per_thread);
    // }

    size_t lcallcount = 0;

    auto lambda = [&](size_t thread_id, size_t cell_id, size_t neighbor_id) {
      // lcallcount++;
      auto&& cell = (*cells)[cell_id];
      const auto&& neighbor = (*cells)[neighbor_id];
      std::array<double, 3> neighbor_force;
      neighbor.GetForceOn(cell.GetMassLocation(), cell.GetDiameter(),
                          &neighbor_force);

      auto idx = thread_id * cell_id;
      force[idx][0] += neighbor_force[0];
      force[idx][1] += neighbor_force[1];
      force[idx][2] += neighbor_force[2];

      idx = thread_id * neighbor_id;
      force[idx][0] += neighbor_force[0];
      force[idx][1] += neighbor_force[1];
      force[idx][2] += neighbor_force[2];

      // OmpAtomicAdd(force[cell_id][0], neighbor_force[0]);
      // OmpAtomicAdd(force[cell_id][1], neighbor_force[1]);
      // OmpAtomicAdd(force[cell_id][2], neighbor_force[2]);
      //
      // OmpAtomicAdd(force[neighbor_id][0], neighbor_force[0]);
      // OmpAtomicAdd(force[neighbor_id][1], neighbor_force[1]);
      // OmpAtomicAdd(force[neighbor_id][2], neighbor_force[2]);

      // AtomicDoubleAdd(force[cell_id][0], neighbor_force[0]);
      // AtomicDoubleAdd(force[cell_id][1], neighbor_force[1]);
      // AtomicDoubleAdd(force[cell_id][2], neighbor_force[2]);
      //
      // AtomicDoubleAdd(force[neighbor_id][0], neighbor_force[0]);
      // AtomicDoubleAdd(force[neighbor_id][1], neighbor_force[1]);
      // AtomicDoubleAdd(force[neighbor_id][2], neighbor_force[2]);

      // NB: writing to force directly could lead to race condition
      // auto& tmp = thread_local_force[thread_id][neighbor_id];
      // tmp[0] += neighbor_force[0];
      // tmp[1] += neighbor_force[1];
      // tmp[2] += neighbor_force[2];
      // thread_local_force[thread_id].emplace_back(make_pair(neighbor_id, neighbor_force));
      // thread_local_force[thread_id][neighbor_id][0] += neighbor_force[0];
      // thread_local_force[thread_id][neighbor_id][1] += neighbor_force[1];
      // thread_local_force[thread_id][neighbor_id][2] += neighbor_force[2];
    };

    // grid.ForEachNeighborPair(lambda);
    grid.ForEachNeighborPairWithinRadius(lambda, *cells, squared_radius);
    std::cout << "lcallcount " << lcallcount << std::endl;

    // reduction of thread local vectors
    #pragma omp parallel for
    for (uint64_t cell_idx = 0; cell_idx < num_threads; cell_idx++) {
      for (uint64_t i = 1; i < num_threads; i++) {
        force[cell_idx][0] += force[cell_idx * i][0];
        force[cell_idx][1] += force[cell_idx * i][1];
        force[cell_idx][2] += force[cell_idx * i][2];
      }
    }

    // reduce temporary neighbor results
    // #pragma omp parallel
    // {
    //   auto thread_id = omp_get_thread_num();
    //   auto elements_per_thread = cells->size() / omp_get_num_threads();
    //   auto start = elements_per_thread * thread_id;
    //   auto end = elements_per_thread * (thread_id + 1);
    //
    //   #pragma omp for
    //   for(size_t i = 0; i < thread_local_force.size(); i++) {
    //     const auto& thread_local_map = thread_local_force[i];
    //     for (const auto& entry : thread_local_map) {
    //       const auto& neighbor_id = entry.first;
    //       if (neighbor_id >= start && neighbor_id < end) {
    //         force[neighbor_id][0] = entry.second[0];
    //         force[neighbor_id][1] = entry.second[1];
    //         force[neighbor_id][2] = entry.second[2];
    //       }
    //     }
    //   }
    // }

#pragma omp parallel for
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
      double norm_of_force = std::sqrt(force[i][0] *
                                           force[i][0] +
                                       force[i][1] *
                                           force[i][1] +
                                       force[i][2] *
                                           force[i][2]);

      // is there enough force to :
      //  - make us biologically move (Tractor) :
      //  - break adherence and make us translate ?
      physical_translation = norm_of_force > cell.GetAdherence();

      double mh = h / cell.GetMass();
      // adding the physics translation (scale by weight) if important enough
      if (physical_translation) {
        // We scale the move with mass and time step
        movement_at_next_step[0] += force[i][0] * mh;
        movement_at_next_step[1] += force[i][1] * mh;
        movement_at_next_step[2] += force[i][2] * mh;

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

    delete[] force;

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
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_NEW_H_
