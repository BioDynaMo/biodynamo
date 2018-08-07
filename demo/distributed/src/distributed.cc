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

#include <chrono>
#include <string>
#include <TBufferFile.h>

#include "distributed.h"

int main(int argc, const char** argv) { return bdm::Simulate(argc, argv); }

bool g_under_ray = false;

namespace {

using namespace bdm;

constexpr int kPlasmaTimeout = 30000;  // 30 seconds in ms.
constexpr int kPlasmaAttempts = 3;  // Try to Get() 3 times.

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
  g_under_ray = true;
  g_local_scheduler_socket_name = local_scheduler_socket_name;
  g_object_store_socket_name = object_store_socket_name;
  g_object_store_manager_socket_name = object_store_manager_socket_name;
  g_simulation_id = std::string(simulation_id, 20);
  g_partitioning_scheme = std::string(partitioning_scheme);
}

extern "C" void bdm_simulate_step(
    int64_t step, int64_t box, bool last_iteration,
    double left, double front, double bottom,
    double right, double back, double top) {
  RaySimulation *simulation = new RaySimulation();
  RayScheduler *scheduler = reinterpret_cast<RayScheduler *>(simulation->GetScheduler());
  scheduler->SimulateStep(step, box, last_iteration, {{left, front, bottom}, {right, back, top}});
  delete simulation;
}

/// Returns a partitioner depending on the scheme in `g_partitioning_scheme`.
std::unique_ptr<Partitioner> CreatePartitioner() {
  int x, y, z;
  if (sscanf(g_partitioning_scheme.c_str(), "%d-%d-%d", &x, &y, &z) != 3 ||
      x < 1 || y < 1 || z < 1) {
    std::cerr << "Invalid partitioning scheme \""
              << g_partitioning_scheme << "\".\n";
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
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(g_simulation_id.data()), 20);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(event.data()), event.size());
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
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(g_simulation_id.data()), 20);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(&step), 8);
  sha256_update(&ctx, reinterpret_cast<const BYTE *>(&box), 8);
  if (surface != SurfaceEnum::kNone) {
    int64_t s = surface;
    sha256_update(&ctx, reinterpret_cast<const BYTE *>(&s), 8);
  }
  std::string hash(SHA256_BLOCK_SIZE, '\x00');
  sha256_final(&ctx, reinterpret_cast<unsigned char *>(&hash[0]));
  return plasma::ObjectID::from_binary(hash.substr(SHA256_BLOCK_SIZE - 20));
}

/// Allocates memory for the main volume, its 6 surfaces, 12 edges, 8 corners.
SurfaceToVolumeMap AllocVolumes() {
  const std::array<Surface, 6> surface_list =
      {SurfaceEnum::kLeft, SurfaceEnum::kFront, SurfaceEnum::kBottom,
       SurfaceEnum::kRight, SurfaceEnum::kBack, SurfaceEnum::kTop};
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
      if (full_surface.conflict(adjacent_surface)) {
        continue;
      }
      ret[i].first = full_surface | adjacent_surface;
      ret[i].second.reset(new ResourceManager<>());
      ++i;
      for (size_t innermost = inner + 1; innermost < surface_list.size(); ++innermost) {
        const Surface &third = surface_list[innermost];
        if (full_surface.conflict(third) || adjacent_surface.conflict(third)) {
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
    const Point3D &this_point,
    const Box &box,
    const std::array<double, 3> &xyz_halos) {
  std::array<Surface, 7> ret;
  // Ensure that the halo is within the region.
  assert(xyz_halos[0] >= 0);
  assert(xyz_halos[1] >= 0);
  assert(xyz_halos[2] >= 0);
  Point3D left_front_bottom = box.first;
  Point3D right_back_top = box.second;
  assert(left_front_bottom[0] + xyz_halos[0] < right_back_top[0]);
  assert(left_front_bottom[1] + xyz_halos[1] < right_back_top[1]);
  assert(left_front_bottom[2] + xyz_halos[2] < right_back_top[2]);

  if (!is_in(this_point, left_front_bottom, right_back_top)) {
    return ret;
  }

  const double left = left_front_bottom[0];
  const double left_plus = left_front_bottom[0] + xyz_halos[0];
  const double right = right_back_top[0];
  const double right_minus = right_back_top[0] - xyz_halos[0];
  const double front = left_front_bottom[1];
  const double front_plus = left_front_bottom[1] + xyz_halos[1];
  const double back = right_back_top[1];
  const double back_minus = right_back_top[1] - xyz_halos[1];
  const double bottom = left_front_bottom[2];
  const double bottom_plus = left_front_bottom[2] + xyz_halos[2];
  const double top = right_back_top[2];
  const double top_minus = right_back_top[2] - xyz_halos[2];
  size_t i = 0;

  if (is_in(this_point, left_front_bottom, {right, front_plus, top})) {
    ret[i++] = SurfaceEnum::kFront;
  }
  if (is_in(this_point, left_front_bottom, {right, front_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kBottom;
  }
  if (is_in(this_point, left_front_bottom, {left_plus, front_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kBottom | SurfaceEnum::kLeft;
  }
  if (is_in(this_point, {right_minus, front, bottom}, {right, front_plus, bottom_plus})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kBottom | SurfaceEnum::kRight;
  }
  if (is_in(this_point, left_front_bottom, {left_plus, front_plus, top})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kLeft;
  }
  if (is_in(this_point, {left, front, top_minus}, {left_plus, front_plus, top})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kLeft | SurfaceEnum::kTop;
  }
  if (is_in(this_point, {left, front, top_minus}, {right, front_plus, top})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kTop;
  }
  if (is_in(this_point, {right_minus, front, top_minus}, {right, front_plus, top})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kTop | SurfaceEnum::kRight;
  }
  if (is_in(this_point, {right_minus, front, bottom}, {right, front_plus, top})) {
    ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kRight;
  }
  if (is_in(this_point, {left, front, top_minus}, right_back_top)) {
    ret[i++] = SurfaceEnum::kTop;
  }
  if (is_in(this_point, {left, front, top_minus}, {left_plus, back, top})) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kLeft;
  }
  if (is_in(this_point, {left, back_minus, top_minus}, {left_plus, back, top})) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kLeft | SurfaceEnum::kBack;
  }
  if (is_in(this_point, {right_minus, front, top_minus}, right_back_top)) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kRight;
  }
  if (is_in(this_point, {right_minus, back_minus, top_minus}, right_back_top)) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kRight | SurfaceEnum::kBack;
  }
  if (is_in(this_point, {left, back_minus, top_minus}, right_back_top)) {
    ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kBack;
  }
  if (is_in(this_point, {left, back_minus, bottom}, right_back_top)) {
    ret[i++] = SurfaceEnum::kBack;
  }
  if (is_in(this_point, {left, back_minus, bottom}, {left_plus, back, top})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kLeft;
  }
  if (is_in(this_point, {right_minus, back_minus, bottom}, right_back_top)) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kRight;
  }
  if (is_in(this_point, {left, back_minus, bottom}, {right, back, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kBottom;
  }
  if (is_in(this_point, {left, back_minus, bottom}, {left_plus, back, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kBottom | SurfaceEnum::kLeft;
  }
  if (is_in(this_point, {right_minus, back_minus, bottom}, {right, back, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kBottom | SurfaceEnum::kRight;
  }
  if (is_in(this_point, left_front_bottom, {right, back, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBottom;
  }
  if (is_in(this_point, left_front_bottom, {left_plus, back, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBottom | SurfaceEnum::kLeft;
  }
  if (is_in(this_point, {right_minus, front, bottom}, {right, back, bottom_plus})) {
    ret[i++] = SurfaceEnum::kBottom | SurfaceEnum::kRight;
  }
  if (is_in(this_point, left_front_bottom, {left_plus, back, top})) {
    ret[i++] = SurfaceEnum::kLeft;
  }
  if (is_in(this_point, {right_minus, front, bottom}, right_back_top)) {
    ret[i++] = SurfaceEnum::kRight;
  }
  assert(i <= ret.size());
  return ret;
}

/// Returns the ResourceManager for the specified surface.
ResourceManagerPtr FindResourceManager(
    const SurfaceToVolumeMap &map,
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

SurfaceToVolumeMap CreateVolumesForBox(
    ResourceManager<> *rm,
    const Box &box) {
  SurfaceToVolumeMap ret = AllocVolumes();
  auto f = [&](const auto &element, bdm::SoHandle) {
    Point3D pos = element.GetPosition();
    if (is_in(pos, box)) {
      ResourceManagerPtr volume_rm = ret[0].second;
      volume_rm->push_back(element);
      for (Surface s : FindContainingSurfaces(
          pos, box, {1, 1, 1})) {
        if (s == SurfaceEnum::kNone) {
          break;
        }
        ResourceManagerPtr surface_rm = FindResourceManager(ret, s);
        surface_rm->push_back(element);
      }
    }
  };
  rm->ApplyOnAllElements(f);
  return ret;
};

}  // namespace

namespace bdm {

void RayScheduler::InitiallyPartition(Box *boundingBox) {
  std::cout << "In RayScheduler::InitiallyPartition\n";
  Simulation<> *sim = Simulation<>::GetActive();
  ResourceManager<> *rm = sim->GetResourceManager();
  std::cout << "Total " << rm->GetNumSimObjects() << '\n';

  std::unique_ptr<Partitioner> partitioner = CreatePartitioner();
  partitioner->InitializeWithResourceManager(rm);
  Boxes boxes = partitioner->Partition();
  for (size_t i = 0; i < boxes.size(); ++i) {
    const Box &box = boxes[i];
    SurfaceToVolumeMap volumes = CreateVolumesForBox(rm, box);
    ResourceManagerPtr main_rm = FindResourceManager(volumes, Surface());
    std::cout << "Box " << i
              << " has " << main_rm->GetNumSimObjects()
              << " simulation objects.\n";
    arrow::Status s = StoreVolumes(0, i, volumes);
    if (!s.ok()) {
      std::cerr << "Cannot store box " << i << ".\n";
      return;
    }
  }

  if (boundingBox != nullptr) {
    *boundingBox = partitioner->GetBoundingBox();
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
  json_stream
      << "{\n"
      << "  \"steps\": " << steps << ",\n"
      << "  \"bounding_box\": [" << std::setprecision(9) << std::fixed
      << bound.first[0] << ", " << bound.first[1] << ", " << bound.first[2] << ", "
      << bound.second[0] << ", " << bound.second[1] << ", " << bound.second[2] << "]\n"
      << "}";
  std::string json = json_stream.str();
  std::shared_ptr<Buffer> buffer;
  plasma::ObjectID start_key = id_for_event(kSimulationStartMarker);
  s = object_store_.Create(start_key,
                           json.size(),
                           nullptr,
                           0,
                           &buffer);
  if (!s.ok()) {
    std::cerr << "Cannot create simulation start marker. " << s <<
              " Simulation aborted\n";
    return;
  }
  memcpy(buffer->mutable_data(), json.data(), json.size());
  s = object_store_.Seal(start_key);
  if (!s.ok()) {
    std::cerr << "Cannot seal simulation start marker. " << s <<
              " Simulation aborted\n";
    return;
  }

  std::cout << "Waiting for end of simulation...\n";
  plasma::ObjectID end_key = id_for_event(kSimulationEndMarker);
  plasma::ObjectRequest wait_request;
  wait_request.object_id = end_key;
  wait_request.type = plasma::ObjectRequestType::PLASMA_QUERY_ANYWHERE;
  int _ignored;
  s = object_store_.Wait(
      1, &wait_request, 1, std::numeric_limits<int64_t>::max(), &_ignored);
  if (!s.ok()) {
    std::cerr << "Error waiting for simulation end marker. " << s << '\n';
    return;
  }
}

/// Partitions `rm` and stores them.
void RayScheduler::DisassembleResourceManager(
    ResourceManager<>* rm, const Partitioner* partitioner,
    int64_t step, int64_t box) {
  auto start = std::chrono::high_resolution_clock::now();
  arrow::Status s = StoreVolumes(
      step, box, CreateVolumesForBox(rm, partitioner->GetLocation(box)));
  if (!s.ok()) {
    std::cerr << "Cannot store volumes for box " << box << " in step " << step << ".\n";
  } else {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Disassemble time " << elapsed.count() << '\n';
  }
}

void RayScheduler::SimulateStep(
    int64_t step, int64_t box, bool last_iteration, const Box &bound) {
  MaybeInitializeConnection();
  std::unique_ptr<Partitioner> partitioner(CreatePartitioner());
  partitioner->InitializeWithBoundingBox(bound.first, bound.second);
  ResourceManager<> *rm = ReassembleVolumes(step, box, partitioner.get());
  std::cout << "Box " << box << " has " << rm->GetNumSimObjects()
            << " simulation objects.\n";
  RaySimulation *sim = reinterpret_cast<RaySimulation *>(Simulation<>::GetActive());
  sim->ReplaceResourceManager(rm);
  Execute(last_iteration);
  DisassembleResourceManager(rm, partitioner.get(), step + 1, box);
}

arrow::Status RayScheduler::MaybeInitializeConnection() {
  if (initialized_) {
    return arrow::Status();
  }
  local_scheduler_.reset(LocalSchedulerConnection_init(
      g_local_scheduler_socket_name.c_str(),
      UniqueID::from_random(),
      false,
      false
  ));
  if (!local_scheduler_) {
    std::cerr << "Cannot create new local scheduler connection to \""
              << g_local_scheduler_socket_name
              << "\"\n";
    return arrow::Status(arrow::StatusCode::IOError, "Cannot connect");
  }
  arrow::Status s = object_store_.Connect(
      g_object_store_socket_name.c_str(),
      g_object_store_manager_socket_name.c_str());
  if (!s.ok()) {
    std::cerr << "Cannot connect to object store (\""
              << g_object_store_socket_name
              << "\", \""
              << g_object_store_manager_socket_name
              << "\"). " << s << '\n';
    return s;
  }
  initialized_ = true;
  return arrow::Status();
}

arrow::Status RayScheduler::StoreVolumes(
    int64_t step, int64_t box, const SurfaceToVolumeMap &volumes) {
  for (const SurfaceToVolume &sv : volumes) {
    Surface surface = sv.first;
    ResourceManagerPtr rm = sv.second;
    TBufferFile buff(TBufferFile::EMode::kWrite);
    buff.WriteObject(rm.get());
    const size_t size = buff.BufferSize();
    const plasma::ObjectID key = id_for_surface(step, box, surface);
    std::shared_ptr<Buffer> buffer;
    arrow::Status s = object_store_.Create(
        key,
        size,
        nullptr,
        0,
        &buffer);
    if (!s.ok()) {
      std::cerr << "Cannot push volume surface " << static_cast<long>(surface)
                << " for box " << box << " in step "
                << step << ". " << s << '\n';
      return s;
    }
    memcpy(buffer->mutable_data(), buff.Buffer(), size);
    s = object_store_.Seal(key);
    if (!s.ok()) {
      std::cerr << "Cannot seal box " << box << " in step "
                << step << ". " << s << '\n';
      return s;
    }
  }

  return arrow::Status();
}

ResourceManager<> *RayScheduler::ReassembleVolumes(
    int64_t step, int64_t box, const Partitioner* partitioner) {
  auto start = std::chrono::high_resolution_clock::now();

  // First create an RM for the main volume.
  plasma::ObjectID key = id_for_surface(step, box, SurfaceEnum::kNone);
  std::vector<plasma::ObjectBuffer> buffers = FetchAndGetVolume(key);
  if (buffers.empty()) {
    std::cerr << "Cannot fetch and get volume for step " << step << " from "
              << box << ".\n";
    return nullptr;
  }

  ResourceManager<> *ret = nullptr;
  TBufferFile f(TBufferFile::EMode::kRead, buffers[0].data->size(),
                const_cast<uint8_t *>(buffers[0].data->data()), false);
  ret = reinterpret_cast<ResourceManager<> *>(f.ReadObjectAny(ResourceManager<>::Class()));

  // Then add from the border regions.
  for (const auto &ns : partitioner->GetNeighborSurfaces(box)) {
    arrow::Status s = AddFromVolume(ret, step, ns.first, ns.second);
    if (!s.ok()) {
      delete ret;
      std::cerr << "Cannot add halos in step " << step << " from box 1.\n";
      return nullptr;
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Reassemble time " << elapsed.count() << '\n';

  return ret;
}

arrow::Status RayScheduler::AddFromVolume(ResourceManager<> *rm, int64_t step, int64_t box, Surface surface) {
  plasma::ObjectID key = id_for_surface(step, box, surface);
  std::vector<plasma::ObjectBuffer> buffers = FetchAndGetVolume(key);
  if (buffers.empty()) {
    std::cerr << "Cannot fetch and get volume for step " << step << " from "
              << box << ".\n";
    return arrow::Status(arrow::StatusCode::IOError, "Cannot fetch and get");
  }
  ResourceManagerPtr subvolume_rm;
  TBufferFile f(TBufferFile::EMode::kRead, buffers[0].data->size(),
                const_cast<uint8_t *>(buffers[0].data->data()), false);
  subvolume_rm.reset(reinterpret_cast<ResourceManager<> *>(
                         f.ReadObjectAny(ResourceManager<>::Class())));

  auto func = [&](const auto &element, bdm::SoHandle) {
    rm->push_back(element);
  };

  subvolume_rm->ApplyOnAllElements(func);

  return arrow::Status();
}

RaySimulation::RaySimulation()
    : super(plasma::ObjectID::from_binary(g_simulation_id).hex()) {}

}  // namespace bdm
