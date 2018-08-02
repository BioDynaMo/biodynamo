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

#include <string>
#include "distributed.h"

int main(int argc, const char** argv) { return bdm::Simulate(argc, argv); }

using namespace bdm;
constexpr Surface SurfaceEnum::kNone;
constexpr Surface SurfaceEnum::kLeft;
constexpr Surface SurfaceEnum::kRight;
constexpr Surface SurfaceEnum::kTop;
constexpr Surface SurfaceEnum::kBottom;
constexpr Surface SurfaceEnum::kFront;
constexpr Surface SurfaceEnum::kBack;

std::string g_local_scheduler_socket_name;
std::string g_object_store_socket_name;
std::string g_object_store_manager_socket_name;
std::string g_simulation_id;
std::string g_partitioning_scheme;

extern "C" void bdm_setup_ray(const char *local_scheduler_socket_name,
                              const char *object_store_socket_name,
                              const char *object_store_manager_socket_name,
                              const char *simulation_id,
                              const char *partitioning_scheme) {
  g_local_scheduler_socket_name = local_scheduler_socket_name;
  g_object_store_socket_name = object_store_socket_name;
  g_object_store_manager_socket_name = object_store_manager_socket_name;
  g_simulation_id = std::string(simulation_id, 20);
  g_partitioning_scheme = std::string(partitioning_scheme);
}

extern "C" void simulate_step(int64_t step, int64_t node, bool last_iteration,
    double left, double front, double bottom, double right, double back, double top) {
  RaySimulation* simulation = new RaySimulation();
  RayScheduler* scheduler = reinterpret_cast<RayScheduler*>(simulation->GetScheduler());
  scheduler->SimulateStep(step, node, last_iteration, {{left, front, bottom}, {right, back, top}});
  delete simulation;
}

void RayScheduler::SimulateStep(long step, long node, bool last_iteration, const Box& bound) {
  Initialize();
  ResourceManager<>* rm = ReassembleVolumes(step, node, bound);
  std::cout << "Box " << node << " has " << rm->GetNumSimObjects()
            << " simulation objects.\n";
  RaySimulation* sim = reinterpret_cast<RaySimulation*>(Simulation<>::GetActive());
  sim->ReplaceResourceManager(rm);
  Execute(last_iteration);
  std::unique_ptr<Partitioner> partitioner(CreatePartitioner());
  partitioner->InitializeWithBoundingBox(bound.first, bound.second);
  arrow::Status s = StoreVolumes(
      step + 1, node, CreateVolumesForBox(rm, partitioner->GetLocation(node)));
  if (!s.ok()) {
    std::cerr << "Cannot store volumes for node " << node << " in step " << step << ".\n";
  }
}

ResourceManager<>* RayScheduler::ReassembleVolumes(int64_t step, int64_t node, const Box& bound) {
  // First create an RM for the main volume.
  plasma::ObjectID key = plasma::ObjectID::from_binary(hash_volume_surface(
      step, node, SurfaceEnum::kNone));
  std::vector<plasma::ObjectBuffer> buffers = FetchAndGetVolume(key);
  if (buffers.empty()) {
    std::cerr << "Cannot fetch and get volume for step " << step << " from "
              << node << ".\n";
    return nullptr;
  }

  ResourceManager<>* ret = nullptr;
  TBufferFile f(TBufferFile::EMode::kRead, buffers[0].data->size(),
      const_cast<uint8_t*>(buffers[0].data->data()), false);
  ret = reinterpret_cast<ResourceManager<>*>(f.ReadObjectAny(ResourceManager<>::Class()));
  object_store_.Release(key);

  // Then add from the border regions.
  std::unique_ptr<Partitioner> partitioner(CreatePartitioner());
  partitioner->InitializeWithBoundingBox(bound.first, bound.second);
  for (const auto& ns : partitioner->GetNeighborSurfaces(node)) {
    arrow::Status s = AddFromVolume(ret, step, ns.first, ns.second);
    if (!s.ok()) {
      delete ret;
      std::cerr << "Cannot add halos in step " << step << " from box 1.\n";
      return nullptr;
    }
  }

  return ret;
}

arrow::Status RayScheduler::AddFromVolume(ResourceManager<>* rm, long step, long node, Surface surface) {
  plasma::ObjectID key = plasma::ObjectID::from_binary(hash_volume_surface(
      step, node, surface));
  std::vector<plasma::ObjectBuffer> buffers = FetchAndGetVolume(key);
  if (buffers.empty()) {
    std::cerr << "Cannot fetch and get volume for step " << step << " from "
              << node << ".\n";
    return arrow::Status(arrow::StatusCode::IOError, "Cannot fetch and get");
  }
  ResourceManagerPtr subvolume_rm;
  TBufferFile f(TBufferFile::EMode::kRead, buffers[0].data->size(),
                const_cast<uint8_t*>(buffers[0].data->data()), false);
  subvolume_rm.reset(reinterpret_cast<ResourceManager<>*>(
      f.ReadObjectAny(ResourceManager<>::Class())));
  object_store_.Release(key);

  auto func = [&](const auto& element, bdm::SoHandle) {
    rm->push_back(element);
  };

  subvolume_rm->ApplyOnAllElements(func);

  return arrow::Status();
}
