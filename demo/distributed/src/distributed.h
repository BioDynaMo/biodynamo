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

#include <memory>
#include <TBufferFile.h>

#include "biodynamo.h"
#include "common/event_loop.h"
#include "local_scheduler/local_scheduler_client.h"
#include "plasma/client.h"
#include "sha256.h"

extern std::string g_local_scheduler_socket_name;
extern std::string g_object_store_socket_name;
extern std::string g_object_store_manager_socket_name;
extern std::string g_simulation_id;

constexpr char kSimulationStartMarker[] = "aaaaaaaaaaaaaaaaaaaa";
constexpr char kSimulationEndMarker[] = "bbbbbbbbbbbbbbbbbbbb";

namespace bdm {

// -----------------------------------------------------------------------------
// This model creates a grid of 128x128x128 cells. Each cell grows untill a
// specific volume, after which it proliferates (i.e. divides).
// -----------------------------------------------------------------------------

// 1. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  // use predefined biology module GrowDivide
  using BiologyModules = Variant<GrowDivide>;
  // use default Backend and AtomicTypes
  using SimulationBackend = Scalar;
};

class Surface {
 public:
  Surface() : Surface(0) {}

  constexpr Surface intersect(const Surface& s) const {
    return Surface(value_ |  s.value_);
  }

  constexpr bool operator==(const Surface& other) const {
    return value_ == other.value_;
  }

  constexpr Surface operator|(const Surface& other) const {
    return intersect(other);
  }

  constexpr bool conflict(const Surface& other) const {
    return (value_ == 1 && other.value_ == 8) ||
        (value_ == 2 && other.value_ == 16) ||
        (value_ == 4 && other.value_ == 32) ||
        (value_ == 8 && other.value_ == 1) ||
        (value_ == 16 && other.value_ == 4) ||
        (value_ == 32 && other.value_ == 8);
  }

  constexpr operator long() const {
    return value_;
  }

 private:
  constexpr explicit Surface(int v) : value_(v) {}
  int value_;
  friend class SurfaceEnum;
};

class SurfaceEnum {
 public:
  static constexpr Surface kNone{0};
  static constexpr Surface kLeft{1};
  static constexpr Surface kFront{2};
  static constexpr Surface kBottom{4};
  static constexpr Surface kRight{8};
  static constexpr Surface kBack{16};
  static constexpr Surface kTop{32};
};

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

using Point3D = std::array<double, 3>;
using Box = std::pair<Point3D, Point3D>;
using BoxId = int;
using Boxes = std::vector<Box>;

/// Returns true if pos is in a bounded box.
///
/// /param pos the location to check
/// /param left_front_bottom (inclusive)
/// /param right_back_top (exclusive)
static inline bool is_in(const Point3D& pos,
                         const Point3D& left_front_bottom,
                         const Point3D& right_back_top) {
  double x = pos[0];
  double y = pos[1];
  double z = pos[2];
  return x >= left_front_bottom[0] && x < right_back_top[0] &&
      y >= left_front_bottom[1] && y < right_back_top[1] &&
      z >= left_front_bottom[2] && z < right_back_top[2];
}

static inline bool is_in(const Point3D& pos,
                         const Box& box) {
  return is_in(pos, box.first, box.second);
}

using NeighborSurface = std::pair<BoxId, Surface>;
using NeighborSurfaces = std::vector<NeighborSurface>;

class Partitioner {
 public:
  constexpr Partitioner(
      const Point3D &left_front_bottom,
      const Point3D &right_back_top)
      : left_front_bottom_(left_front_bottom), right_back_top_(right_back_top) {}

  /// Constructs a partitioner with simulation objects contained in rm.
  Partitioner(ResourceManager<>* rm) {
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::min();
    double min_z = std::numeric_limits<double>::max();
    double max_z = std::numeric_limits<double>::min();
    auto f = [&](auto element, bdm::SoHandle) {
      std::array<double, 3> pos = element.GetPosition();
      min_x = std::min(min_x, pos[0]);
      max_x = std::max(max_x, pos[0]);
      min_y = std::min(min_y, pos[1]);
      max_y = std::max(max_y, pos[1]);
      min_z = std::min(min_z, pos[2]);
      max_z = std::max(max_z, pos[2]);
    };
    // Could parallel this min/max finding.
    rm->ApplyOnAllElements(f);
    max_x += 1e-9;
    max_y += 1e-9;
    max_z += 1e-9;
    left_front_bottom_ = {min_x, min_y, min_z};
    right_back_top_ = {max_x, max_y, max_z};
  }

  virtual ~Partitioner() {}

  /// Partitions the given box into smaller ones.
  virtual Boxes Partition() const;

  /// Retrieves the coordinates of the box indexed by boxIndex.
  ///
  /// This is basically the same as Partition()[boxIndex] but may be more
  /// optimized in concrete partitioner implementations.
  virtual Box GetLocation(BoxId boxIndex) const {
    assert(boxIndex >= 0);
    Boxes boxes = Partition();
    assert(boxIndex < boxes.size());
    return boxes[boxIndex];
  }

  /// Returns the box index containing point.
  virtual BoxId Locate(Point3D point) const;

  /// Returns all surfaces surrounding the box indexed by boxIndex.
  ///
  /// There could be 6 full surfaces, and 12 edges neighboring a box.
  virtual NeighborSurfaces GetNeighborSurfaces(BoxId boxIndex) const;

 protected:
  Point3D left_front_bottom_;
  Point3D right_back_top_;
};

class HypercubePartitioner : public Partitioner {
 public:
  using super = Partitioner;
  HypercubePartitioner(
      const Point3D &left_front_bottom,
      const Point3D &right_back_top,
      int power)
      : super(left_front_bottom, right_back_top), power_(power) {
    // Only support 2 boxes at the moment.
    assert(power == 1);
    mid_x_ = left_front_bottom_[0] + (right_back_top_[0] - left_front_bottom_[0]) / 2;
  }

  HypercubePartitioner(ResourceManager<>* rm, int power)
      : super(rm), power_(power) {
    assert(power == 1);
    mid_x_ = left_front_bottom_[0] + (right_back_top_[0] - left_front_bottom_[0]) / 2;
  }

  virtual Boxes Partition() const override {
    return {
        GetLocation(0),
        GetLocation(1),
    };
  }

  virtual Box GetLocation(BoxId boxIndex) const override {
    assert(boxIndex >= 0 && boxIndex <= 1);
    if (boxIndex == 0) {
      return {left_front_bottom_, {mid_x_, right_back_top_[1], right_back_top_[2]}};
    }
    return {{mid_x_, left_front_bottom_[1], left_front_bottom_[2]}, right_back_top_};
  }

  virtual BoxId Locate(Point3D point) const override {
    assert(is_in(point, left_front_bottom_, right_back_top_));
    if (point[0] < mid_x_) {
      return 0;
    }
    return 1;
  }

  virtual NeighborSurfaces GetNeighborSurfaces(BoxId boxIndex) const override {
    assert(boxIndex >= 0 && boxIndex <= 1);
    if (boxIndex == 0) {
      return {{1, SurfaceEnum::kLeft}};
    }
    return {{0, SurfaceEnum::kRight}};
  }
 private:
  int power_;
  double mid_x_;
};

using ResourceManagerPtr = std::shared_ptr<ResourceManager<>>;
using SurfaceToVolume = std::pair<Surface, ResourceManagerPtr>;

class RayScheduler : public Scheduler<Simulation<>> {
 public:
  using super = Scheduler<Simulation<>>;

  void SimulateStep(long step, long node, bool last_iteration);

  /// Initiates a distributed simulation and waits for its completion.
  ///
  /// This method will:
  ///
  /// #. RAIIs necessary Ray resources such as object store, local scheduler.
  /// #. Initially distributes the cells to volumes via Plasma objects.
  ///    Each main volume will be accompanied by 6 + 12 = 18 halo (margin)
  ///    volumes. Each of the volume will have a determined ID.
  /// #. Put the number of steps to the kSimulationStartMarker object.
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
    Partition();

    std::shared_ptr<Buffer> buffer;
    s = object_store_.Create(plasma::ObjectID::from_binary(kSimulationStartMarker),
                             sizeof(steps),
                             nullptr,
                             0,
                             &buffer);
    if (!s.ok()) {
      std::cerr << "Cannot create simulation start marker. " << s <<
                   " Simulation aborted\n";
      return;
    }
    memcpy(buffer->mutable_data(), &steps, sizeof(steps));
    s = object_store_.Seal(plasma::ObjectID::from_binary(kSimulationStartMarker));
    if (!s.ok()) {
      std::cerr << "Cannot seal simulation start marker. " << s <<
                   " Simulation aborted\n";
      return;
    }
    s = object_store_.Release(plasma::ObjectID::from_binary(kSimulationStartMarker));
    if (!s.ok()) {
      std::cerr << "Cannot release simulation start marker. " << s <<
                   " Simulation aborted\n";
      return;
    }

    std::vector<plasma::ObjectBuffer> _ignored;
    std::cout << "Waiting for end of simulation...\n";
    s = object_store_.Get({plasma::ObjectID::from_binary(kSimulationEndMarker)},
                          -1,
                          &_ignored);
    if (!s.ok()) {
      std::cerr << "Error waiting for simulation end marker. " << s << '\n';
      return;
    }
  }

  /// Allocates memory for the main volume, its 6 surfaces, and 12 edges.
  static std::array<SurfaceToVolume, 19> AllocVolumes() {
    const std::array<Surface, 6> surface_list =
        {SurfaceEnum::kLeft, SurfaceEnum::kFront, SurfaceEnum::kBottom,
         SurfaceEnum::kRight, SurfaceEnum::kBack, SurfaceEnum::kTop};
    std::array<SurfaceToVolume, 19> ret;
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
      }
    }
    assert(i == ret.size());
    return ret;
  }

  /// Returns a list of border surfaces that this_point belongs to.
  ///
  /// A point may belong to 1 to 3 of the 6 surfaces, and/or some of the 12
  /// edges (could be 0, 1, 2, or 3 edges).
  ///
  /// \param this_point the point location that we want to find surfaces for
  /// \param box the (left_front_bottom, right_back_top) coordinates
  /// \param xyz_halos the margins corresponding to x-, y-, and z-axis
  /// \return list of Surfaces, terminating in Surface::kNone
  static std::array<Surface, 6> FindContainingSurfaces(
      const Point3D& this_point,
      const Box& box,
      const std::array<double, 3>& xyz_halos) {
    std::array<Surface, 6> ret;
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
    int i = 0;
    if (is_in(this_point, left_front_bottom, {
        right_back_top[0], left_front_bottom[1] + xyz_halos[1], right_back_top[2]})) {
      ret[i++] = SurfaceEnum::kFront;
    }
    if (is_in(this_point, left_front_bottom, {
        right_back_top[0], left_front_bottom[1] + xyz_halos[1], left_front_bottom[2] + xyz_halos[2]})) {
      ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kBottom;
    }
    if (is_in(this_point, left_front_bottom, {
        left_front_bottom[0] + xyz_halos[0], left_front_bottom[1] + xyz_halos[1], right_back_top[2]})) {
      ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kLeft;
    }
    if (is_in(this_point, {left_front_bottom[0], left_front_bottom[1], right_back_top[2] - xyz_halos[2]},
              {right_back_top[0], left_front_bottom[1] + xyz_halos[1], right_back_top[2]})) {
      ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kTop;
    }
    if (is_in(this_point, {right_back_top[0] - xyz_halos[0], left_front_bottom[1], left_front_bottom[2]},
              {right_back_top[0], left_front_bottom[1] + xyz_halos[1], right_back_top[2]})) {
      ret[i++] = SurfaceEnum::kFront | SurfaceEnum::kRight;
    }
    if (is_in(this_point, {left_front_bottom[0], left_front_bottom[1], right_back_top[2] - xyz_halos[2]},
              {right_back_top})) {
      ret[i++] = SurfaceEnum::kTop;
    }
    if (is_in(this_point, {left_front_bottom[0], left_front_bottom[1], right_back_top[2] - xyz_halos[2]},
              {left_front_bottom[0] + xyz_halos[0], right_back_top[1], right_back_top[2]})) {
      ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kLeft;
    }
    if (is_in(this_point, {right_back_top[0] - xyz_halos[0], left_front_bottom[1], right_back_top[2] - xyz_halos[2]},
              right_back_top)) {
      ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kRight;
    }
    if (is_in(this_point, {left_front_bottom[0], right_back_top[1] - xyz_halos[1], right_back_top[2] - xyz_halos[2]},
              right_back_top)) {
      ret[i++] = SurfaceEnum::kTop | SurfaceEnum::kBack;
    }
    if (is_in(this_point, {left_front_bottom[0], right_back_top[1] - xyz_halos[1], left_front_bottom[2]},
              right_back_top)) {
      ret[i++] = SurfaceEnum::kBack;
    }
    if (is_in(this_point, {left_front_bottom[0], right_back_top[1] - xyz_halos[1], left_front_bottom[2]},
              {left_front_bottom[0] + xyz_halos[0], right_back_top[1], right_back_top[2]})) {
      ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kLeft;
    }
    if (is_in(this_point, {right_back_top[0] - xyz_halos[0], right_back_top[1] - xyz_halos[1], left_front_bottom[2]},
              right_back_top)) {
      ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kRight;
    }
    if (is_in(this_point, {left_front_bottom[0], right_back_top[1] - xyz_halos[1], left_front_bottom[2]},
              {right_back_top[0], right_back_top[1], left_front_bottom[2] + xyz_halos[2]})) {
      ret[i++] = SurfaceEnum::kBack | SurfaceEnum::kBottom;
    }
    if (is_in(this_point, left_front_bottom,
              {right_back_top[0], right_back_top[1], left_front_bottom[2] + xyz_halos[2]})) {
      ret[i++] = SurfaceEnum::kBottom;
    }
    if (is_in(this_point, left_front_bottom,
              {left_front_bottom[0] + xyz_halos[0], right_back_top[1], left_front_bottom[2] + xyz_halos[2]})) {
      ret[i++] = SurfaceEnum::kBottom | SurfaceEnum::kLeft;
    }
    if (is_in(this_point, {right_back_top[0] - xyz_halos[0], left_front_bottom[1], left_front_bottom[2]},
              {right_back_top[0], right_back_top[1], left_front_bottom[2] + xyz_halos[2]})) {
      ret[i++] = SurfaceEnum::kBottom | SurfaceEnum::kRight;
    }
    if (is_in(this_point, left_front_bottom,
              {left_front_bottom[0] + xyz_halos[0], right_back_top[1], right_back_top[2]})) {
      ret[i++] = SurfaceEnum::kLeft;
    }
    if (is_in(this_point, {right_back_top[0] - xyz_halos[0], left_front_bottom[1], left_front_bottom[2]},
              right_back_top)) {
      ret[i++] = SurfaceEnum::kRight;
    }
    assert(i <= 6);
    return ret;
  }

  /// Returns the ResourceManager for the specified surface.
  static ResourceManagerPtr FindResourceManager(
      const std::array<SurfaceToVolume, 19>& map,
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

  std::array<SurfaceToVolume, 19> CreateVolumesForBox(
      ResourceManager<>* rm,
      const Box& box) {
    std::array<SurfaceToVolume, 19> ret = AllocVolumes();
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

  arrow::Status StorePartition(
      long step,
      long box,
      const std::array<SurfaceToVolume, 19>& node) {
    for (const SurfaceToVolume& sv : node) {
      Surface surface = sv.first;
      ResourceManagerPtr rm = sv.second;
      TBufferFile buff(TBufferFile::EMode::kWrite);
      buff.WriteObject(rm.get());
      const size_t size = buff.BufferSize();
      std::string hash = hash_volume_surface(step, box, surface);
      plasma::ObjectID key = plasma::ObjectID::from_binary(hash);
      std::shared_ptr<Buffer> buffer;
      arrow::Status s = object_store_.Create(
          key,
          size,
          nullptr,
          0,
          &buffer);
      if (!s.ok()) {
        std::cerr << "Cannot push volume for box " << box << " in step "
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
      s = object_store_.Release(key);
      if (!s.ok()) {
        std::cerr << "Cannot release box " << box << " in step "
                  << step << ". " << s << '\n';
        return s;
      }
    }
    return arrow::Status();
  }

  ResourceManager<>* ReassembleVolumes(int64_t step, int64_t node);

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

  arrow::Status AddFromVolume(ResourceManager<>* rm, long step, long node, Surface surface);

  /// Partitions cells into 3D volumes and their corresponding halo volumes.
  ///
  /// The results of the partitioning are stored in the object store directly.
  virtual void Partition() {
    std::cout << "In RayScheduler::Partition\n";
    Simulation<> *sim = Simulation<>::GetActive();
    ResourceManager<> *rm = sim->GetResourceManager();

    HypercubePartitioner partitioner(rm, 1);
    std::array<SurfaceToVolume, 19> node_1 = CreateVolumesForBox(
        rm, partitioner.GetLocation(0));
    std::array<SurfaceToVolume, 19> node_2 = CreateVolumesForBox(
        rm, partitioner.GetLocation(1));
    std::cout << "Total " << rm->GetNumSimObjects() << '\n';
    std::cout << "Node 1 " << node_1[0].second->GetNumSimObjects() << '\n';
    std::cout << "Node 2 " << node_2[0].second->GetNumSimObjects() << '\n';

    arrow::Status s = StorePartition(0, 0, node_1);
    if (!s.ok()) {
      std::cerr << "Cannot store partition 0.\n";
      return;
    }
    s = StorePartition(0, 1, node_2);
    if (!s.ok()) {
      std::cerr << "Cannot store partition 1.\n";
      return;
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
    cell.SetDiameter(30);
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
