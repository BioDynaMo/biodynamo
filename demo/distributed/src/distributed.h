// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef DEMO_DISTRIBUTED_DISTRIBUTED_H_
#define DEMO_DISTRIBUTED_DISTRIBUTED_H_

#include <cassert>
#include <memory>
#include <TBufferFile.h>

#include "backend.h"
#include "local_scheduler/local_scheduler_client.h"
#include "partitioner.h"
#include "plasma/client.h"
#include "sha256.h"

constexpr char kSimulationStartMarker[] = "aaaaaaaaaaaaaaaaaaaa";
constexpr char kSimulationEndMarker[] = "bbbbbbbbbbbbbbbbbbbb";

namespace bdm {

using ResourceManagerPtr = std::shared_ptr<ResourceManager<>>;
using SurfaceToVolume = std::pair<Surface, ResourceManagerPtr>;
// Not a map, but a constant size linear array.
using SurfaceToVolumeMap = std::array<SurfaceToVolume, 27>;

class RayScheduler : public Scheduler<Simulation<>> {
 public:
  using super = Scheduler<Simulation<>>;

  /// Runs one simulation timestep for `box` in `step` with global `bound`.
  void SimulateStep(long step, long box, bool last_iteration, const Box& bound);

  /// Initiates a distributed simulation and waits for its completion.
  ///
  /// This method will:
  ///
  /// #. RAIIs necessary Ray resources such as object store, local scheduler.
  /// #. Initially distributes the cells to volumes via Plasma objects.
  ///    Each main volume will be accompanied by 6 + 12 + 8 = 26 halo (margin)
  ///    volumes. Each of the volume will have a determined ID.
  /// #. Put the number of steps and bounding box to the kSimulationStartMarker
  ///    object.
  /// #. Waits for the kSimulationEndMarker object.
  ///
  /// From the Python side, it will take the number of steps, construct a chain
  /// of remote calls to actually run each step based on the ID of the regions,
  /// and finally mark the end of the simulation.
  ///
  /// \param steps number of steps to simulate.
  virtual void Simulate(uint64_t steps) override;

  virtual ~RayScheduler() {
  }

 private:
  arrow::Status MaybeInitializeConnection();

  arrow::Status StoreVolumes(
      long step,
      long box,
      const SurfaceToVolumeMap& volumes);

  /// Add all simulation objects from `box`'s `surface` in `step` to `rm`.
  arrow::Status AddFromVolume(ResourceManager<>* rm, long step, long box, Surface surface);

  /// Reassembles all volumes required to simulate `box` in `step`.
  ResourceManager<>* ReassembleVolumes(long step, long box, const Box& bound);

  /// Calls Plasma `Fetch` and `Get` on `key`.
  std::vector<plasma::ObjectBuffer> FetchAndGetVolume(
      const plasma::ObjectID& key);

  /// Partitions cells into 3D volumes and their corresponding halo volumes.
  ///
  /// The results of the partitioning are stored in the object store directly.
  ///
  /// \param boundingBox output argument to receive the bounding box of the world
  virtual void InitiallyPartition(Box* boundingBox);

  bool initialized_ = false;
  std::unique_ptr<LocalSchedulerConnection> local_scheduler_ = nullptr;
  plasma::PlasmaClient object_store_;
};

class RaySimulation : public Simulation<> {
 public:
  using super = Simulation<>;
  RaySimulation();
  RaySimulation(int argc, const char **argv) : super(argc, argv) {}
  virtual ~RaySimulation() {}
  virtual Scheduler<Simulation>* GetScheduler() override {
    if (!scheduler_set_) {
      ReplaceScheduler(new RayScheduler());
      scheduler_set_ = true;
    }
    return super::GetScheduler();
  }
  virtual void ReplaceResourceManager(ResourceManager<>* rm) {
    rm_ = rm;
  }
 private:
  bool scheduler_set_ = false;
};

inline int Simulate(int argc, const char** argv) {
  // 2. Create new simulation
  RaySimulation simulation(argc, argv);

  // 3. Define initial model - in this example: 3D grid of cells
  size_t cells_per_dim = 16;
  auto construct = [](const std::array<double, 3> &position) {
    Cell cell(position);
    cell.SetDiameter(10);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(GrowDivide());
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 20, construct);

  // 4. Run simulation for one timestep
  simulation.GetScheduler()->Simulate(42);

  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm
#endif  // DEMO_DISTRIBUTED_DISTRIBUTED_H_
