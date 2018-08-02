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

extern std::string g_local_scheduler_socket_name;
extern std::string g_object_store_socket_name;
extern std::string g_object_store_manager_socket_name;
extern std::string g_simulation_id;
extern std::string g_partitioning_scheme;

constexpr char kSimulationStartMarker[] = "aaaaaaaaaaaaaaaaaaaa";
constexpr char kSimulationEndMarker[] = "bbbbbbbbbbbbbbbbbbbb";

namespace bdm {

static std::string hash_event(std::string event) {
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, reinterpret_cast<const BYTE*>(g_simulation_id.data()), 20);
  sha256_update(&ctx, reinterpret_cast<const BYTE*>(event.data()), event.size());
  std::string hash(SHA256_BLOCK_SIZE, '\x00');
  sha256_final(&ctx, reinterpret_cast<unsigned char*>(&hash[0]));
  return hash.substr(SHA256_BLOCK_SIZE - 20);
}

static std::string hash_volume_surface(long step, long box,
                                       Surface surface = SurfaceEnum::kNone) {
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, reinterpret_cast<const BYTE*>(g_simulation_id.data()), 20);
  sha256_update(&ctx, reinterpret_cast<const BYTE*>(&step), 8);
  sha256_update(&ctx, reinterpret_cast<const BYTE*>(&box), 8);
  if (surface != SurfaceEnum::kNone) {
    long s = surface;
    sha256_update(&ctx, reinterpret_cast<const BYTE *>(&s), 8);
  }
  std::string hash(SHA256_BLOCK_SIZE, '\x00');
  sha256_final(&ctx, reinterpret_cast<unsigned char*>(&hash[0]));
  return hash.substr(SHA256_BLOCK_SIZE - 20);
}

static std::unique_ptr<Partitioner> CreatePartitioner() {
  if (g_partitioning_scheme == "2-1-1") {
    return std::unique_ptr<Partitioner>(new CubePartitioner({2, 1, 1}));
  } else if (g_partitioning_scheme == "3-3-3") {
    return std::unique_ptr<Partitioner>(new CubePartitioner({3, 3, 3}));
  }
  // Must never happen.
  assert(false);
  return nullptr;
}

using ResourceManagerPtr = std::shared_ptr<ResourceManager<>>;
using SurfaceToVolume = std::pair<Surface, ResourceManagerPtr>;

class RayScheduler : public Scheduler<Simulation<>> {
 public:
  using super = Scheduler<Simulation<>>;

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
  virtual void Simulate(uint64_t steps) override {
    arrow::Status s = Initialize();
    if (!s.ok()) {
      std::cerr << "Cannot make connection to local scheduler or object store. "
                << s << " Simulation aborted.\n";
      return;
    }

    Box bound;
    Partition(&bound);

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
    plasma::ObjectID start_key = plasma::ObjectID::from_binary(
        hash_event(kSimulationStartMarker));
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
    plasma::ObjectID end_key = plasma::ObjectID::from_binary(
        hash_event(kSimulationEndMarker));
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

  /// Allocates memory for the main volume, its 6 surfaces, 12 edges, 8 corners.
  static std::array<SurfaceToVolume, 27> AllocVolumes() {
    const std::array<Surface, 6> surface_list =
        {SurfaceEnum::kLeft, SurfaceEnum::kFront, SurfaceEnum::kBottom,
         SurfaceEnum::kRight, SurfaceEnum::kBack, SurfaceEnum::kTop};
    std::array<SurfaceToVolume, 27> ret;
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
  static std::array<Surface, 7> FindContainingSurfaces(
      const Point3D& this_point,
      const Box& box,
      const std::array<double, 3>& xyz_halos) {
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
  static ResourceManagerPtr FindResourceManager(
      const std::array<SurfaceToVolume, 27>& map,
      Surface s) {
    for (const SurfaceToVolume& entry : map) {
      if (entry.first == s) {
        return entry.second;
      }
    }
    // This must never happen.
    assert(false);
    return nullptr;
  }

  std::array<SurfaceToVolume, 27> CreateVolumesForBox(
      ResourceManager<>* rm,
      const Box& box) {
    std::array<SurfaceToVolume, 27> ret = AllocVolumes();
    auto f = [&](const auto& element, bdm::SoHandle) {
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

  arrow::Status StoreVolumes(
      long step,
      long box,
      const std::array<SurfaceToVolume, 27>& volumes) {
    for (const SurfaceToVolume& sv : volumes) {
      Surface surface = sv.first;
      ResourceManagerPtr rm = sv.second;
      TBufferFile buff(TBufferFile::EMode::kWrite);
      buff.WriteObject(rm.get());
      const size_t size = buff.BufferSize();
      std::string hash = hash_volume_surface(step, box, surface);
      const plasma::ObjectID key = plasma::ObjectID::from_binary(hash);
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

  ResourceManager<>* ReassembleVolumes(long step, long box, const Box& bound);

  std::vector<plasma::ObjectBuffer> FetchAndGetVolume(const plasma::ObjectID& key) {
    arrow::Status s;
    s = object_store_.Fetch(1, &key);
    if (!s.ok()) {
      std::cerr << "Cannot fetch \"" << key.hex() << "\". " << s << '\n';
      return {};
    }
    std::vector<plasma::ObjectBuffer> buffers;
    s = object_store_.Get({key}, -1, &buffers);
    if (!s.ok()) {
      std::cerr << "Cannot get \"" << key.hex() << "\". " << s << '\n';
      return buffers;
    }
    return buffers;
  }

  arrow::Status AddFromVolume(ResourceManager<>* rm, long step, long box, Surface surface);

  /// Partitions cells into 3D volumes and their corresponding halo volumes.
  ///
  /// The results of the partitioning are stored in the object store directly.
  ///
  /// \param boundingBox output argument to receive the bounding box of the world
  virtual void Partition(Box* boundingBox) {
    std::cout << "In RayScheduler::Partition\n";
    Simulation<> *sim = Simulation<>::GetActive();
    ResourceManager<> *rm = sim->GetResourceManager();
    std::cout << "Total " << rm->GetNumSimObjects() << '\n';

    std::unique_ptr<Partitioner> partitioner = CreatePartitioner();
    partitioner->InitializeWithResourceManager(rm);
    Boxes boxes = partitioner->Partition();
    for (size_t i = 0; i < boxes.size(); ++i) {
      const Box& box = boxes[i];
      std::array<SurfaceToVolume, 27> volumes = CreateVolumesForBox(rm, box);
      std::cout << "Box " << i
                << " has " << volumes[0].second->GetNumSimObjects()
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

  virtual ~RayScheduler() {
  }

 private:
  arrow::Status Initialize() {
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

  bool initialized_ = false;
  std::unique_ptr<LocalSchedulerConnection> local_scheduler_ = nullptr;
  plasma::PlasmaClient object_store_;
};

class RaySimulation : public Simulation<> {
 public:
  using super = Simulation<>;
  RaySimulation() : super(plasma::ObjectID::from_binary(g_simulation_id).hex()) {}
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
