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

#include <TBufferFile.h>
#include <chrono>
#include <limits>
#include <string>
#include <vector>

#include "distributed.h"

int main(int argc, const char **argv) { return bdm::Simulate(argc, argv); }

bool g_under_ray = false;

namespace bdm {

constexpr int kPlasmaTimeout = 30000;  // 30 seconds in ms.
constexpr int kPlasmaAttempts = 3;     // Try to Get() 3 times.

std::string g_local_scheduler_socket_name;
std::string g_object_store_socket_name;
std::string g_object_store_manager_socket_name;
std::string g_simulation_id;
std::string g_partitioning_scheme;

Box g_global_box = {
    {
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max()
    },
    {
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min()
    }
};
Point3D g_halo_xyz = {30, 30, 30};

extern "C" void bdm_setup_ray(const char *local_scheduler_socket_name,
                              const char *object_store_socket_name,
                              const char *object_store_manager_socket_name,
                              const char *simulation_id,
                              const char *partitioning_scheme) {
  g_under_ray = true;
  g_local_scheduler_socket_name = local_scheduler_socket_name;
  g_object_store_socket_name = object_store_socket_name;
  g_object_store_manager_socket_name = object_store_manager_socket_name;
  g_simulation_id = std::string(simulation_id, 20);
  g_partitioning_scheme = std::string(partitioning_scheme);
}

extern "C" void bdm_simulate_step(int64_t step, int64_t box,
                                  bool last_iteration, double left,
                                  double front, double bottom, double right,
                                  double back, double top) {
  RaySimulation *simulation = new RaySimulation();
  RayScheduler *scheduler =
      reinterpret_cast<RayScheduler *>(simulation->GetScheduler());
  scheduler->SimulateStep(step, box, last_iteration,
                          {{left, front, bottom}, {right, back, top}});
  delete simulation;
}

extern "C" void bdm_set_global_space(
    double left, double front, double bottom,
    double right, double back, double top,
    double x, double y, double z) {
  assert(x >= 0);
  assert(y >= 0);
  assert(z >= 0);
  assert(left < right);
  assert(front < back);
  assert(bottom < top);
  assert(left + 2 * x < right);
  assert(front + 2 * y < back);
  assert(bottom + 2 * z < top);
  g_global_box = {{left, front, bottom}, {right, back, top}};
  g_halo_xyz = {x, y, z};
}

/// Returns a partitioner depending on the scheme in `g_partitioning_scheme`.
std::unique_ptr<Partitioner> CreatePartitioner() {
  int x, y, z;
  if (sscanf(g_partitioning_scheme.c_str(), "%d-%d-%d", &x, &y, &z) != 3 ||
      x < 1 || y < 1 || z < 1) {
    std::cerr << "Invalid partitioning scheme \"" << g_partitioning_scheme
              << "\".\n";
    return nullptr;
  }
  return std::unique_ptr<Partitioner>(new CubePartitioner({x, y, z}));
}

extern "C" int bdm_get_box_count() {
  std::unique_ptr<Partitioner> partitioner = CreatePartitioner();
  return partitioner->GetBoxCount();
}

extern "C" int bdm_get_neighbor_surfaces(int box, int *out) {
  std::unique_ptr<Partitioner> partitioner = CreatePartitioner();
  int i = 0;
  for (NeighborSurface ns : partitioner->GetNeighborSurfaces(box)) {
    out[i++] = ns.first;
    out[i++] = ns.second;
  }
  return i / 2;
}

/// Returns an ObjectID for `event` under `g_simulation_id` namespace.
plasma::ObjectID id_for_event(std::string event) {
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(g_simulation_id.data()),
                20);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(event.data()),
                event.size());
  std::string hash(SHA256_BLOCK_SIZE, '\x00');
  sha256_final(&ctx, reinterpret_cast<unsigned char *>(&hash[0]));
  return plasma::ObjectID::from_binary(hash.substr(SHA256_BLOCK_SIZE - 20));
}

/// Returns an ObjectID for `step`, `box` and `surface` under `g_simulation_id`
/// namespace.
plasma::ObjectID id_for_surface(int64_t step, int64_t box,
                                Surface surface = SurfaceEnum::kNone) {
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(g_simulation_id.data()),
                20);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(&step), 8);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(&box), 8);
  int64_t s = surface;
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(&s), 8);
  std::string hash(SHA256_BLOCK_SIZE, '\x00');
  sha256_final(&ctx, reinterpret_cast<unsigned char *>(&hash[0]));
  return plasma::ObjectID::from_binary(hash.substr(SHA256_BLOCK_SIZE - 20));
}

/// Allocates memory for the main volume, its 6 surfaces, 12 edges, 8 corners.
SurfaceToVolumeMap AllocVolumes() {
  const std::array<Surface, 6> surface_list = {
      SurfaceEnum::kLeft,  SurfaceEnum::kFront, SurfaceEnum::kBottom,
      SurfaceEnum::kRight, SurfaceEnum::kBack,  SurfaceEnum::kTop};
  SurfaceToVolumeMap ret;
  ret[0].second.reset(new ResourceManager<>());
  size_t i = 1;
  for (size_t outer = 0; outer < surface_list.size(); ++outer) {
    const Surface &full_surface = surface_list[outer];
    ret[i].first = full_surface;
    ret[i].second.reset(new ResourceManager<>());
    ++i;
    for (size_t inner = outer + 1; inner < surface_list.size(); ++inner) {
      const Surface &adjacent_surface = surface_list[inner];
      if (full_surface.Conflict(adjacent_surface)) {
        continue;
      }
      ret[i].first = full_surface | adjacent_surface;
      ret[i].second.reset(new ResourceManager<>());
      ++i;
      for (size_t innermost = inner + 1; innermost < surface_list.size();
           ++innermost) {
        const Surface &third = surface_list[innermost];
        if (full_surface.Conflict(third) || adjacent_surface.Conflict(third)) {
          continue;
        }
        ret[i].first = full_surface | adjacent_surface | third;
        ret[i].second.reset(new ResourceManager<>());
        i++;
      }
    }
  }
  assert(i == ret.size());
  return ret;
}

/// Returns a list of border surfaces that this_point belongs to.
///
/// A point may belong to at most 3 of the 6 surfaces, at most 3 of the 12
/// edges, and at most 1 corner.
///
/// \param this_point the point location that we want to find surfaces for
/// \param box the (left_front_bottom, right_back_top) coordinates
/// \param xyz_halos the margins corresponding to x-, y-, and z-axis
/// \return list of Surfaces, terminating in Surface::kNone
std::array<Surface, 7> FindContainingSurfaces(
    const Point3D &this_point, const Box &box,
    const std::array<double, 3> &xyz_halos) {
  std::array<Surface, 7> ret;
  // Ensure that the halo is within the region, and non-overlapping.
  assert(xyz_halos[0] >= 0);
  assert(xyz_halos[1] >= 0);
  assert(xyz_halos[2] >= 0);
  Point3D left_front_bottom = box.first;
  Point3D right_back_top = box.second;
  assert(left_front_bottom[0] < right_back_top[0]);
  assert(left_front_bottom[1] < right_back_top[1]);
  assert(left_front_bottom[2] < right_back_top[2]);
  assert(left_front_bottom[0] + 2 * xyz_halos[0] < right_back_top[0]);
  assert(left_front_bottom[1] + 2 * xyz_halos[1] < right_back_top[1]);
  assert(left_front_bottom[2] + 2 * xyz_halos[2] < right_back_top[2]);

  const double left = left_front_bottom[0];
  const double left_minus = left - xyz_halos[0];
  const double left_plus = left + xyz_halos[0];
  const double right = right_back_top[0];
  const double right_minus = right - xyz_halos[0];
  const double right_plus = right + xyz_halos[0];
  const double front = left_front_bottom[1];
  const double front_minus = front - xyz_halos[1];
  const double front_plus = front + xyz_halos[1];
  const double back = right_back_top[1];
  const double back_minus = back - xyz_halos[1];
  const double back_plus = back + xyz_halos[1];
  const double bottom = left_front_bottom[2];
  const double bottom_minus = bottom - xyz_halos[2];
  const double bottom_plus = bottom + xyz_halos[2];
  const double top = right_back_top[2];
  const double top_minus = top - xyz_halos[2];
  const double top_plus = top + xyz_halos[2];
  size_t i = 0;

  if (IsIn(this_point,
           {left_minus, front_minus, bottom_minus},
           {right_plus, front_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kFront;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, bottom_minus},
           {right_plus, front_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kBottom;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, bottom_minus},
           {left_plus, front_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kBottom | SurfaceEnum::kLeft;
  }
  if (IsIn(this_point,
           {right_minus, front_minus, bottom_minus},
           {right_plus, front_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kBottom | SurfaceEnum::kRight;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, bottom_minus},
           {left_plus, front_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kLeft;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, top_minus},
           {left_plus, front_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kLeft | SurfaceEnum::kTop;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, top_minus},
           {right_plus, front_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kTop;
  }
  if (IsIn(this_point,
           {right_minus, front_minus, top_minus},
           {right_plus, front_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kTop | SurfaceEnum::kRight;
  }
  if (IsIn(this_point,
           {right_minus, front_minus, bottom_minus},
           {right_plus, front_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kRight;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, top_minus},
           {right_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kTop;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, top_minus},
           {left_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kLeft;
  }
  if (IsIn(this_point,
           {left_minus, back_minus, top_minus},
           {left_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kLeft | SurfaceEnum::kBack;
  }
  if (IsIn(this_point,
           {right_minus, front_minus, top_minus},
           {right_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kRight;
  }
  if (IsIn(this_point,
           {right_minus, back_minus, top_minus},
           {right_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kRight | SurfaceEnum::kBack;
  }
  if (IsIn(this_point,
           {left_minus, back_minus, top_minus},
           {right_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kBack;
  }
  if (IsIn(this_point,
           {left_minus, back_minus, bottom_minus},
           {right_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kBack;
  }
  if (IsIn(this_point,
           {left_minus, back_minus, bottom_minus},
           {left_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kLeft;
  }
  if (IsIn(this_point,
           {right_minus, back_minus, bottom_minus},
           {right_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kRight;
  }
  if (IsIn(this_point,
           {left_minus, back_minus, bottom_minus},
           {right_plus, back_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kBottom;
  }
  if (IsIn(this_point,
           {left_minus, back_minus, bottom_minus},
           {left_plus, back_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kBottom | SurfaceEnum::kLeft;
  }
  if (IsIn(this_point,
           {right_minus, back_minus, bottom_minus},
           {right_plus, back_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kBottom | SurfaceEnum::kRight;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, bottom_minus},
           {right_plus, back_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBottom;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, bottom_minus},
           {left_plus, back_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBottom | SurfaceEnum::kLeft;
  }
  if (IsIn(this_point,
           {right_minus, front_minus, bottom_minus},
           {right_plus, back_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBottom | SurfaceEnum::kRight;
  }
  if (IsIn(this_point,
           {left_minus, front_minus, bottom_minus},
           {left_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kLeft;
  }
  if (IsIn(this_point,
           {right_minus, front_minus, bottom_minus},
           {right_plus, back_plus, top_plus})) {
    ret[i++] = SurfaceEnum::kRight;
  }
  assert(i <= ret.size());
  return ret;
}

/// Returns the ResourceManager for the specified surface.
ResourceManagerPtr FindResourceManager(const SurfaceToVolumeMap &map,
                                       Surface s) {
  for (const SurfaceToVolume &entry : map) {
    if (entry.first == s) {
      return entry.second;
    }
  }
  // This must never happen.
  assert(false);
  return nullptr;
}

SurfaceToVolumeMap RayScheduler::CreateVolumesForBox(
    ResourceManager<> *rm, BoxId box_id, const Partitioner* partitioner,
    bool with_migrations) {
  SurfaceToVolumeMap ret = AllocVolumes();
  std::array<Surface, 27> needed_surfaces =
      partitioner->GetRequiredSurfaces(box_id);
  const Box box = partitioner->GetLocation(box_id);
  StepContext type_counts;
  auto f = [&](const auto &element, bdm::SoHandle handle) {
    const auto type_idx = handle.GetTypeIdx();
    const auto cell_idx = type_counts.IncrementCount(handle.GetTypeIdx());
    // We do not care about cells that were originally outside of the main box.
    if (!step_context_.ShouldManage(type_idx, cell_idx)) {
      return;
    }
    const Point3D pos = element.GetPosition();
    const bool is_in_box = IsIn(pos, box.first, box.second);
    // We do not care about cells outside the box if no migrations.
    if (!with_migrations && !is_in_box) {
      return;
    }
    // The main box.
    if (is_in_box) {
      ResourceManagerPtr volume_rm = ret[0].second;
      volume_rm->push_back(element);
    }
    // And the halo regions.
    for (Surface s : FindContainingSurfaces(pos, box, g_halo_xyz)) {
      if (s == SurfaceEnum::kNone) {
        break;
      }
      size_t i = 0;
      while (needed_surfaces[i] != SurfaceEnum::kNone &&
          needed_surfaces[i] != s) {
        ++i;
      }
      if (needed_surfaces[i] == s) {
        ResourceManagerPtr surface_rm = FindResourceManager(ret, s);
        surface_rm->push_back(element);
      }
    }
  };
  rm->ApplyOnAllElements(f);
  return ret;
}

void RayScheduler::InitiallyPartition(Box *bounding_box) {
  std::cout << "In RayScheduler::InitiallyPartition\n";
  Simulation<> *sim = Simulation<>::GetActive();
  ResourceManager<> *rm = sim->GetResourceManager();
  step_context_.SetCounts(rm);
  std::cout << "Total " << rm->GetNumSimObjects() << '\n';

  std::unique_ptr<Partitioner> partitioner = CreatePartitioner();
  if (g_global_box.first[0] >= g_global_box.second[0]) {
    partitioner->InitializeWithResourceManager(rm);
  } else {
    partitioner->InitializeWithBoundingBox(
        g_global_box.first, g_global_box.second);
  }
  Boxes boxes = partitioner->Partition();
  for (size_t i = 0; i < boxes.size(); ++i) {
    SurfaceToVolumeMap volumes =
        CreateVolumesForBox(rm, i, partitioner.get(),
            /* with_migrations */ false);
    ResourceManagerPtr main_rm = FindResourceManager(volumes, Surface());
    std::cout << "Box " << i << " has " << main_rm->GetNumSimObjects()
              << " simulation objects.\n";
    arrow::Status s = StoreVolumes(0, i, partitioner.get(), volumes);
    if (!s.ok()) {
      std::cerr << "Cannot store box " << i << ".\n";
      return;
    }
  }

  if (bounding_box != nullptr) {
    *bounding_box = partitioner->GetBoundingBox();
  }
}

std::vector<plasma::ObjectBuffer> RayScheduler::FetchAndGetVolume(
    const plasma::ObjectID &key) {
  arrow::Status s;
  s = object_store_.Fetch(1, &key);
  if (!s.ok()) {
    std::cerr << "Cannot fetch \"" << key.hex() << "\". " << s << '\n';
    return {};
  }
  std::vector<plasma::ObjectBuffer> buffers;
  int attempts = 0;
  while (attempts < kPlasmaAttempts) {
    s = object_store_.Get({key}, kPlasmaTimeout, &buffers);
    if (!s.ok() || buffers.empty() || buffers[0].data == nullptr) {
      std::cerr << "Trying to fetch and get " << key.hex() << " again.\n";
      ++attempts;
      buffers.clear();
      continue;
    }
    return buffers;
  }
  std::cerr << "Cannot get \"" << key.hex() << "\". " << s << '\n';
  return {};
}

void RayScheduler::Simulate(uint64_t steps) {
  arrow::Status s = MaybeInitializeConnection();
  if (!s.ok()) {
    std::cerr << "Cannot make connection to local scheduler or object store. "
              << s << " Simulation aborted.\n";
    return;
  }

  Box bound;
  InitiallyPartition(&bound);

  std::ostringstream json_stream;
  json_stream << "{\n"
              << "  \"steps\": " << steps << ",\n"
              << "  \"bounding_box\": [" << std::setprecision(9) << std::fixed
              << bound.first[0] << ", " << bound.first[1] << ", "
              << bound.first[2] << ", " << bound.second[0] << ", "
              << bound.second[1] << ", " << bound.second[2] << "]\n"
              << "}";
  std::string json = json_stream.str();
  std::shared_ptr<Buffer> buffer;
  plasma::ObjectID start_key = id_for_event(kSimulationStartMarker);
  s = object_store_.Create(start_key, json.size(), nullptr, 0, &buffer);
  if (!s.ok()) {
    std::cerr << "Cannot create simulation start marker. " << s
              << " Simulation aborted\n";
    return;
  }
  memcpy(buffer->mutable_data(), json.data(), json.size());
  s = object_store_.Seal(start_key);
  if (!s.ok()) {
    std::cerr << "Cannot seal simulation start marker. " << s
              << " Simulation aborted\n";
    return;
  }

  std::cout << "Waiting for end of simulation...\n";
  plasma::ObjectID end_key = id_for_event(kSimulationEndMarker);
  plasma::ObjectRequest wait_request;
  wait_request.object_id = end_key;
  wait_request.type = plasma::ObjectRequestType::PLASMA_QUERY_ANYWHERE;
  int _ignored;
  s = object_store_.Wait(1, &wait_request, 1,
                         std::numeric_limits<int64_t>::max(), &_ignored);
  if (!s.ok()) {
    std::cerr << "Error waiting for simulation end marker. " << s << '\n';
    return;
  }
}

/// Partitions `rm` and stores them.
void RayScheduler::DisassembleResourceManager(ResourceManager<> *rm,
                                              const Partitioner *partitioner,
                                              int64_t step, int64_t box) {
  auto start = std::chrono::high_resolution_clock::now();
  std::cout << "Disassemble input " << box << " has "
            << rm->GetNumSimObjects() << ".\n";
  SurfaceToVolumeMap volumes = CreateVolumesForBox(
      rm, box, partitioner, /* with_migrations */ true);
  std::cout << "Disassemble main box " << box << " has "
            << volumes[0].second->GetNumSimObjects() << ".\n";
  arrow::Status s = StoreVolumes(step, box, partitioner, volumes);
  if (!s.ok()) {
    std::cerr << "Cannot store volumes for box " << box << " in step " << step
              << ".\n";
  } else {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Disassemble time " << elapsed.count() << " ms\n";
  }
}

void RayScheduler::SimulateStep(int64_t step, int64_t box, bool last_iteration,
                                const Box &bound) {
  MaybeInitializeConnection();
  std::unique_ptr<Partitioner> partitioner(CreatePartitioner());
  partitioner->InitializeWithBoundingBox(bound.first, bound.second);
  ResourceManager<> *rm = ReassembleVolumes(step, box, partitioner.get());
  std::cout << "Reassemble output " << box << " has " << rm->GetNumSimObjects()
            << " simulation objects.\n";
  RaySimulation *sim =
      reinterpret_cast<RaySimulation *>(Simulation<>::GetActive());
  sim->ReplaceResourceManager(rm);
  auto start = std::chrono::high_resolution_clock::now();
  Execute(last_iteration);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Execute time " << elapsed.count() << " ms\n";
  DisassembleResourceManager(rm, partitioner.get(), step + 1, box);
}

arrow::Status RayScheduler::MaybeInitializeConnection() {
  if (initialized_) {
    return arrow::Status();
  }
  local_scheduler_.reset(
      LocalSchedulerConnection_init(g_local_scheduler_socket_name.c_str(),
                                    UniqueID::from_random(), false, false));
  if (!local_scheduler_) {
    std::cerr << "Cannot create new local scheduler connection to \""
              << g_local_scheduler_socket_name << "\"\n";
    return arrow::Status(arrow::StatusCode::IOError, "Cannot connect");
  }
  arrow::Status s =
      object_store_.Connect(g_object_store_socket_name.c_str(),
                            g_object_store_manager_socket_name.c_str());
  if (!s.ok()) {
    std::cerr << "Cannot connect to object store (\""
              << g_object_store_socket_name << "\", \""
              << g_object_store_manager_socket_name << "\"). " << s << '\n';
    return s;
  }
  initialized_ = true;
  return arrow::Status();
}

arrow::Status RayScheduler::StoreVolumes(int64_t step, int64_t box,
                                         const Partitioner* partitioner,
                                         const SurfaceToVolumeMap &volumes) {
  std::array<Surface, 27> needed_surfaces =
      partitioner->GetRequiredSurfaces(box);
  for (const Surface& surface : needed_surfaces) {
    ResourceManagerPtr rm = FindResourceManager(volumes, surface);
    TBufferFile buff(TBufferFile::EMode::kWrite);
    buff.WriteObjectAny(rm.get(), ResourceManager<>::Class());
    const size_t size = buff.BufferSize();
    const plasma::ObjectID key = id_for_surface(step, box, surface);
    std::shared_ptr<Buffer> buffer;
    arrow::Status s = object_store_.Create(key, size, nullptr, 0, &buffer);
    if (!s.ok()) {
      std::cerr << "Cannot push volume surface "
                << static_cast<int64_t>(surface) << " for box " << box
                << " in step " << step << ". " << s << '\n';
      return s;
    }
    memcpy(buffer->mutable_data(), buff.Buffer(), size);
    s = object_store_.Seal(key);
    if (!s.ok()) {
      std::cerr << "Cannot seal box " << box << " in step " << step << ". " << s
                << '\n';
      return s;
    }
    s = object_store_.Release(key);
    if (!s.ok()) {
      std::cerr << "Cannot release box " << box << " in step " << step << ". "
                << s << '\n';
      return s;
    }
    if (surface == SurfaceEnum::kNone) {
      break;
    }
  }

  return arrow::Status();
}

ResourceManager<> *RayScheduler::ReassembleVolumes(
    int64_t step, int64_t box_id, const Partitioner *partitioner) {
  auto start = std::chrono::high_resolution_clock::now();

  // First create an RM for the main volume.
  plasma::ObjectID key = id_for_surface(step, box_id, SurfaceEnum::kNone);
  std::vector<plasma::ObjectBuffer> buffers = FetchAndGetVolume(key);
  if (buffers.empty()) {
    std::cerr << "Cannot fetch and get volume for step " << step << " from "
              << box_id << ".\n";
    return nullptr;
  }

  ResourceManager<> *ret = nullptr;
  TBufferFile f(TBufferFile::EMode::kRead, buffers[0].data->size(),
                const_cast<uint8_t *>(buffers[0].data->data()), false);
  ret = reinterpret_cast<ResourceManager<> *>(
      f.ReadObjectAny(ResourceManager<>::Class()));
  std::cout << "Reassemble main box " << box_id << " has "
            << ret->GetNumSimObjects() << ".\n";

  auto end_main = std::chrono::high_resolution_clock::now();

  // Then add from the border regions. Three steps.
  // First, fetch all halo regions.
  std::array<ResourceManagerPtr, 26> neighbor_rms;
  size_t region_count = 0;
  for (const auto &ns : partitioner->GetNeighborSurfaces(box_id)) {
    plasma::ObjectID key = id_for_surface(step, ns.first, ns.second);
    std::vector<plasma::ObjectBuffer> buffers = FetchAndGetVolume(key);
    if (buffers.empty()) {
      std::cerr << "Cannot fetch and get volume for step " << step << " from "
                << box_id << ".\n";
      delete ret;
      return nullptr;
    }
    ResourceManagerPtr subvolume_rm;
    TBufferFile f(TBufferFile::EMode::kRead, buffers[0].data->size(),
                  const_cast<uint8_t *>(buffers[0].data->data()), false);
    subvolume_rm.reset(reinterpret_cast<ResourceManager<> *>(
                           f.ReadObjectAny(ResourceManager<>::Class())));
    neighbor_rms[region_count++] = subvolume_rm;
  }
  assert(region_count <= neighbor_rms.size());

  // Second, add simulation objects that are in the main region.
  const Box box = partitioner->GetLocation(box_id);
  auto add_main = [&](const auto& element, SoHandle handle) {
    if (IsIn(element.GetPosition(), box.first, box.second)) {
      ret->push_back(element);
    }
  };
  for (size_t i = 0; i < region_count; ++i) {
    ResourceManagerPtr subvolume_rm = neighbor_rms[i];
    subvolume_rm->ApplyOnAllElements(add_main);
  }

  step_context_.SetCounts(ret);

  // Third, add remaining simulation objects.
  auto add_remaining = [&](const auto& element, SoHandle) {
    if (!IsIn(element.GetPosition(), box.first, box.second)) {
      ret->push_back(element);
    }
  };
  for (size_t i = 0; i < region_count; ++i) {
    ResourceManagerPtr subvolume_rm = neighbor_rms[i];
    subvolume_rm->ApplyOnAllElements(add_remaining);
  }

  auto end_all = std::chrono::high_resolution_clock::now();

  std::chrono::milliseconds elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(end_main - start);
  std::cout << "Reassemble without halos time " << elapsed.count() << " ms\n";
  elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(end_all - start);
  std::cout << "Reassemble with halos time " << elapsed.count() << " ms\n";

  return ret;
}

RaySimulation::RaySimulation()
    : super(plasma::ObjectID::from_binary(g_simulation_id).hex()) {}

}  // namespace bdm
