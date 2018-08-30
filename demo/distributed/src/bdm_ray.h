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

#ifndef DEMO_DISTRIBUTED_SRC_BDM_RAY_H_
#define DEMO_DISTRIBUTED_SRC_BDM_RAY_H_

#include <local_scheduler/local_scheduler_client.h>
#include <plasma/client.h>

#include <utility>
#include <vector>

#include "backend.h"
#include "partitioner.h"

constexpr char kSimulationStartMarker[] = "aaaaaaaaaaaaaaaaaaaa";
constexpr char kSimulationEndMarker[] = "bbbbbbbbbbbbbbbbbbbb";

namespace bdm {

using ResourceManagerPtr = std::shared_ptr<ResourceManager<>>;
using SurfaceToVolume = std::pair<Surface, ResourceManagerPtr>;
// Not a map, but a constant size linear array.
using SurfaceToVolumeMap = std::array<SurfaceToVolume, 27>;

/// Stores the context of a simulation step.
///
/// Right now, the stored context only contains the number of simulation objects
/// (SO's) that are originally, at reassembling time, within the box under
/// consideration. These SOs are stored at the beginning of the re-assembled
/// ResourceManager.
class StepContext {
 public:
  /// Constructs a zero'd context.
  StepContext() {
    for (size_t i = 0; i < managed_counts_.size(); ++i) {
      managed_counts_[i] = 0;
    }
  }

  /// Obtains the counts from resource manager `rm`.
  void SetCounts(ResourceManager<>* rm) {
    auto func = [&](const auto& container, auto idx) {
      managed_counts_[idx] = container->size();
    };
    rm->ApplyOnAllTypes(func);
  }

  /// Increments the count for `type_idx` and returns the previous count.
  uint32_t IncrementCount(uint16_t type_idx) {
    return managed_counts_[type_idx]++;
  }

  /// Returns true if the `element_idx` in `type_idx` should be managed.
  bool ShouldManage(uint16_t type_idx, uint32_t element_idx) const {
    return element_idx < managed_counts_[type_idx];
  }

 private:
  using TypeCounts = std::array<uint32_t, ResourceManager<>::NumberOfTypes()>;
  TypeCounts managed_counts_;
};

class RayScheduler : public Scheduler<Simulation<>> {
 public:
  using super = Scheduler<Simulation<>>;

  /// Runs one simulation timestep for `box` in `step` with global `bound`.
  void SimulateStep(int64_t step, int64_t box, bool last_iteration,
                    const Box& bound);

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
  void Simulate(uint64_t steps) override;

  virtual ~RayScheduler() {}

 private:
  /// Establishes connections to Ray's local scheduler and Plasma object store.
  arrow::Status MaybeInitializeConnection();

  /// Copies simulation objects from `rm` to a SurfaceToVolumeMap for `box_id`.
  ///
  /// Only the SOs that are managed by this box (those that *were* originally
  /// inside the box, at reassembling time) may be copied. If `with_migrations`
  /// is on, the managed SOs that have migrated outside the box will be copied.
  ///
  /// \param rm the ResourceManager containing all SOs to be copied. This
  ///           ResourceManager can contain so many more SOs than the actual
  ///           number of managed SOs.
  /// \param box_id the identifier of the main box
  /// \param partitioner the partitioner to find neighboring surfaces of the
  ///                    main box
  /// \param with_migrations if `true`, copy managed SOs that have moved
  ///                        outside the box
  /// \return a map of Surface to ResourceManager
  SurfaceToVolumeMap CreateVolumesForBox(
      ResourceManager<> *rm, BoxId box_id, const Partitioner* partitioner,
      bool with_migrations);

  /// Stores `volumes` in the object store for `box` in `step`.
  arrow::Status StoreVolumes(int64_t step, int64_t box,
                             const Partitioner* partitioner,
                             const SurfaceToVolumeMap& volumes);

  void DisassembleResourceManager(ResourceManager<>* rm,
                                  const Partitioner* partitioner, int64_t step,
                                  int64_t box);

  /// Reassembles all volumes required to simulate `box` in `step` according to
  /// `partitioner`.
  ResourceManager<>* ReassembleVolumes(int64_t step, int64_t box,
                                       const Partitioner* partitioner);

  /// Calls Plasma `Fetch` and `Get` on `key`.
  std::vector<plasma::ObjectBuffer> FetchAndGetVolume(
      const plasma::ObjectID& key);

  /// Partitions cells into 3D volumes and their corresponding halo volumes.
  ///
  /// The results of the partitioning are stored in the object store directly.
  ///
  /// \param bounding_box output argument to receive the bounding box of the
  /// world
  virtual void InitiallyPartition(Box* bounding_box);

  bool initialized_ = false;
  std::unique_ptr<LocalSchedulerConnection> local_scheduler_ = nullptr;
  plasma::PlasmaClient object_store_;
  StepContext step_context_;
};

class RaySimulation : public Simulation<> {
 public:
  using super = Simulation<>;
  RaySimulation();
  RaySimulation(int argc, const char** argv) : super(argc, argv) {}
  virtual ~RaySimulation() {}
  Scheduler<Simulation>* GetScheduler() override {
    if (!scheduler_set_) {
      ReplaceScheduler(new RayScheduler());
      scheduler_set_ = true;
    }
    return super::GetScheduler();
  }
  virtual void ReplaceResourceManager(ResourceManager<>* rm) { rm_ = rm; }

 private:
  bool scheduler_set_ = false;
};

}  // namespace bdm

#endif  // DEMO_DISTRIBUTED_SRC_BDM_RAY_H_
