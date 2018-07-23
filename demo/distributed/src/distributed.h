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

#include "biodynamo.h"
#include "common/event_loop.h"
#include "local_scheduler/local_scheduler_client.h"
#include "plasma/client.h"

extern std::string g_local_scheduler_socket_name;
extern std::string g_object_store_socket_name;
extern std::string g_object_store_manager_socket_name;

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
};

class RayScheduler : public Scheduler<Simulation<>> {
 public:
  using super = Scheduler<Simulation<>>;
  virtual void Simulate(uint64_t steps) override {
    std::cout << "In RayScheduler::Simulate\n";
    conn_ = LocalSchedulerConnection_init(
        g_local_scheduler_socket_name.c_str(),
        UniqueID::from_random(),
        false,
        false
    );
    if (!conn_) {
      std::cerr << "Cannot create new local scheduler connection.\n";
      return;
    }
    plasma.Connect(g_object_store_socket_name.c_str(),
                   g_object_store_manager_socket_name.c_str());
    bool is_in = false;
    plasma::ObjectID id = plasma::UniqueID::from_binary("aaaaaaaaaaaaaaaaaaaa");
    plasma.Contains(id, &is_in);
    std::vector<plasma::ObjectBuffer> buffers;
    std::cout << "Getting the object\n";
    plasma.Get({id}, -1, &buffers);
    std::cout << "Done " << is_in << " size " << buffers.size() << '\n';
    if (buffers.size() > 0) {
      size_t size = buffers[0].data->size();
      for (int i = 0; i < size; ++i) {
        std::cout << buffers[0].data->data()[i];
      }
    }
    std::cout << '\n';
    plasma.Release(id);
    super::Simulate(steps);
  }

  virtual ~RayScheduler() {
    LocalSchedulerConnection_free(conn_);
  }

 private:
  LocalSchedulerConnection *conn_;
  plasma::PlasmaClient plasma;
};

class RaySimulation : public Simulation<> {
 public:
  using super = Simulation<>;
  RaySimulation(int argc, const char **argv) : super(argc, argv) {}
  virtual ~RaySimulation() {}
  virtual Scheduler<Simulation>* GetScheduler() override {
    if (!scheduler_set_) {
      ReplaceScheduler(new RayScheduler());
      scheduler_set_ = true;
    }
    return super::GetScheduler();
  }
 private:
  bool scheduler_set_ = false;
};

inline int Simulate(int argc, const char** argv) {
  // 2. Create new simulation
  RaySimulation simulation(argc, argv);

  // 3. Define initial model - in this example: 3D grid of cells
  size_t cells_per_dim = 128;
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
  simulation.GetScheduler()->Simulate(1);

  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm
#endif  // DEMO_DISTRIBUTED_DISTRIBUTED_H_
