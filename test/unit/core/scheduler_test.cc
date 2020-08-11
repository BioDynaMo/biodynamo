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

#include "unit/core/scheduler_test.h"
#include "core/operation/operation_registry.h"

namespace bdm {
namespace scheduler_test_internal {

#ifdef USE_DICT
TEST(SchedulerTest, NoRestoreFile) {
  auto set_param = [](auto* param) { param->restore_file_ = ""; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  remove(ROOTFILE);

  Cell* cell = new Cell();
  cell->SetDiameter(10);  // important for env to determine box size
  rm->push_back(cell);

  // start restore validation
  TestSchedulerRestore scheduler;
  scheduler.Simulate(100);
  EXPECT_EQ(100u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->GetNumSimObjects());

  scheduler.Simulate(100);
  EXPECT_EQ(200u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->GetNumSimObjects());

  scheduler.Simulate(100);
  EXPECT_EQ(300u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->GetNumSimObjects());
}

TEST(SchedulerTest, Restore) { RunRestoreTest(); }

TEST(SchedulerTest, Backup) { RunBackupTest(); }
#endif  // USE_DICT

TEST(SchedulerTest, EmptySimulationFromBeginning) {
  auto set_param = [](auto* param) {
    param->bound_space_ = true;
    param->min_bound_ = -10;
    param->max_bound_ = 10;
  };
  Simulation simulation(TEST_NAME, set_param);

  simulation.GetScheduler()->Simulate(1);

  auto* env = simulation.GetEnvironment();
  std::array<int32_t, 2> expected_dim_threshold = {-10, 10};
  EXPECT_EQ(expected_dim_threshold, env->GetDimensionThresholds());
  std::array<int32_t, 6> expected_dimensions = {-10, 10, -10, 10, -10, 10};
  EXPECT_EQ(expected_dimensions, env->GetDimensions());
}

TEST(SchedulerTest, EmptySimulationAfterFirstIteration) {
  auto set_param = [](auto* param) {
    param->bound_space_ = true;
    param->min_bound_ = -10;
    param->max_bound_ = 10;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();
  auto* scheduler = simulation.GetScheduler();

  Cell* cell = new Cell(10);
  rm->push_back(cell);
  scheduler->Simulate(1);

  auto max_dimensions = env->GetDimensionThresholds();
  auto dimensions = env->GetDimensions();
  rm->Clear();

  scheduler->Simulate(1);

  EXPECT_EQ(max_dimensions, env->GetDimensionThresholds());
  EXPECT_EQ(dimensions, env->GetDimensions());
  EXPECT_FALSE(env->HasGrown());
}

struct TestOp : public OperationImpl {
  TestOp* Clone() override { return new TestOp(*this); }
  void operator()(SimObject* so) override { counter++; }
  uint64_t counter;

 private:
  static bool registered_;
};

REGISTER_OP(TestOp, "test_op", kCpu)

TEST(SchedulerTest, OperationManagement) {
  Simulation simulation(TEST_NAME);

  simulation.GetResourceManager()->push_back(new Cell(10));

  auto* op1 = GET_OP("test_op");
  auto* op2 = GET_OP("test_op");

  auto* op1_impl = op1->GetImplementation<TestOp>();
  auto* op2_impl = op2->GetImplementation<TestOp>();

  // Change the state of one of the operations
  op1_impl->counter = 1;

  // schedule operations
  auto* scheduler = simulation.GetScheduler();
  scheduler->ScheduleOp(op1);
  scheduler->ScheduleOp(op2);
  scheduler->Simulate(10);
  EXPECT_EQ(11u, op1_impl->counter);
  EXPECT_EQ(10u, op2_impl->counter);

  // change frequency of operation
  op1->frequency_ = 3;
  scheduler->Simulate(10);
  EXPECT_EQ(14u, op1_impl->counter);
  EXPECT_EQ(20u, op2_impl->counter);

  // remove operation
  scheduler->UnscheduleOp(op2);
  scheduler->Simulate(10);
  EXPECT_EQ(17u, op1_impl->counter);
  EXPECT_EQ(20u, op2_impl->counter);

  // get non existing and protected operations
  scheduler->Simulate(10);
  EXPECT_EQ(21u, op1_impl->counter);
  EXPECT_EQ(20u, op2_impl->counter);
}

struct CpuOp : public OperationImpl {
  CpuOp* Clone() override { return new CpuOp(*this); }
  void operator()(SimObject* so) override {}

 private:
  static bool registered_;
};

REGISTER_OP(CpuOp, "cpu_op", kCpu)

struct CudaOp : public OperationImplGpu {
  CudaOp* Clone() override { return new CudaOp(*this); }
  void operator()() override {}

 private:
  static bool registered_;
};

REGISTER_OP(CudaOp, "cuda_op", kCuda)

struct OpenClOp : public OperationImplGpu {
  OpenClOp* Clone() override { return new OpenClOp(*this); }
  void operator()() override {}

 private:
  static bool registered_;
};

REGISTER_OP(OpenClOp, "opencl_op", kOpenCl)

struct MultiOp : public OperationImpl {
  MultiOp* Clone() override { return new MultiOp(*this); }
  void operator()() override {}

 private:
  static bool registered_;
};

REGISTER_OP(MultiOp, "multi_op", kCpu)

struct MultiOpOpenCl : public OperationImplGpu {
  MultiOpOpenCl* Clone() override { return new MultiOpOpenCl(*this); }
  void operator()() override {}

 private:
  static bool registered_;
};

REGISTER_OP(MultiOpOpenCl, "multi_op", kOpenCl)

TEST(SchedulerTest, OperationImpl) {
  auto* cpu_op = GET_OP("cpu_op");
  auto* cuda_op = GET_OP("cuda_op");
  auto* opencl_op = GET_OP("opencl_op");
  auto* multi_op = GET_OP("multi_op");

  auto* cpu_impl = cpu_op->GetImplementation<CpuOp>();
  auto* cuda_impl = cuda_op->GetImplementation<CudaOp>();
  auto* opencl_impl = opencl_op->GetImplementation<OpenClOp>();
  auto* multi_cpu_impl = multi_op->GetImplementation<MultiOp>();
  auto* multi_ocl_impl = multi_op->GetImplementation<MultiOpOpenCl>();

  // Check casting to correct target type
  EXPECT_NE(cpu_impl, nullptr);
  EXPECT_NE(cuda_impl, nullptr);
  EXPECT_NE(opencl_impl, nullptr);
  EXPECT_NE(multi_cpu_impl, nullptr);
  EXPECT_NE(multi_ocl_impl, nullptr);

  // Check resizing and initialization if implementations_ vector
  EXPECT_EQ(cpu_op->implementations_.size(), 1u);
  EXPECT_NE(cpu_op->implementations_[kCpu], nullptr);
  EXPECT_EQ(cuda_op->implementations_.size(), 2u);
  EXPECT_EQ(cuda_op->implementations_[kCpu], nullptr);
  EXPECT_NE(cuda_op->implementations_[kCuda], nullptr);
  EXPECT_EQ(opencl_op->implementations_.size(), 3u);
  EXPECT_EQ(opencl_op->implementations_[kCpu], nullptr);
  EXPECT_EQ(opencl_op->implementations_[kCuda], nullptr);
  EXPECT_NE(opencl_op->implementations_[kOpenCl], nullptr);
  EXPECT_EQ(multi_op->implementations_.size(), 3u);
  EXPECT_NE(multi_op->implementations_[kCpu], nullptr);
  EXPECT_EQ(multi_op->implementations_[kCuda], nullptr);
  EXPECT_NE(multi_op->implementations_[kOpenCl], nullptr);


  EXPECT_EQ(cpu_op->IsComputeTargetSupported(kCpu), true);
  EXPECT_EQ(cpu_op->IsComputeTargetSupported(kCuda), false);
  EXPECT_EQ(cpu_op->IsComputeTargetSupported(kOpenCl), false);
  EXPECT_EQ(cuda_op->IsComputeTargetSupported(kCpu), false);
  EXPECT_EQ(cuda_op->IsComputeTargetSupported(kCuda), true);
  EXPECT_EQ(cuda_op->IsComputeTargetSupported(kOpenCl), false);
  EXPECT_EQ(opencl_op->IsComputeTargetSupported(kCpu), false);
  EXPECT_EQ(opencl_op->IsComputeTargetSupported(kCuda), false);
  EXPECT_EQ(opencl_op->IsComputeTargetSupported(kOpenCl), true);
  EXPECT_EQ(multi_op->IsComputeTargetSupported(kCpu), true);
  EXPECT_EQ(multi_op->IsComputeTargetSupported(kCuda), false);
  EXPECT_EQ(multi_op->IsComputeTargetSupported(kOpenCl), true);

  // Check correctness of target initialization
  EXPECT_EQ(cpu_impl->target_, kCpu);
  EXPECT_EQ(cuda_impl->target_, kCuda);
  EXPECT_EQ(opencl_impl->target_, kOpenCl);
  EXPECT_EQ(multi_cpu_impl->target_, kCpu);
  EXPECT_EQ(multi_ocl_impl->target_, kOpenCl);

  EXPECT_EQ(cpu_impl->IsGpuOperation(), false);
  EXPECT_EQ(cuda_impl->IsGpuOperation(), true);
  EXPECT_EQ(opencl_impl->IsGpuOperation(), true);
  EXPECT_EQ(multi_cpu_impl->IsGpuOperation(), false);
  EXPECT_EQ(multi_ocl_impl->IsGpuOperation(), true);

  EXPECT_EQ(cpu_impl->IsRowWise(), true);
  EXPECT_EQ(cuda_impl->IsRowWise(), false);
  EXPECT_EQ(opencl_impl->IsRowWise(), false);
  EXPECT_EQ(multi_cpu_impl->IsRowWise(), true);
  EXPECT_EQ(multi_ocl_impl->IsRowWise(), false);

  // Try to obtain non-existing implementation
  auto* cpu_op2 = GET_OP("cpu_op");
  auto* cpu_impl2 = cpu_op2->GetImplementation<CudaOp>();
  EXPECT_EQ(cpu_impl2, nullptr);

  // Since we didn't schedule the operations, we are responsible for freeing up
  delete cpu_op;
  delete cpu_op2;
  delete cuda_op;
  delete opencl_op;
  delete multi_op;
}

struct ComplexStateOp : public OperationImpl {
  class A {};
  ComplexStateOp* Clone() override {
    auto* clone = new ComplexStateOp(*this);

    // Deep copy of vector
    int i = 0;
    for (auto* a : a_vec_) {
      clone->a_vec_[i] = new A(*a);
      i++;
    }
    return clone;
  }
  void operator()(SimObject* so) override {}

  bool b_ = false;
  std::vector<A*> a_vec_;

 private:
  static bool registered_;
};

REGISTER_OP(ComplexStateOp, "complex_state_op", kCpu)

TEST(SchedulerTest, OperationCloning) {
  auto* op = GET_OP("complex_state_op");
  op->frequency_ = 42;
  op->active_target_ = kOpenCl;

  auto* op_impl = op->GetImplementation<ComplexStateOp>();
  op_impl->b_ = true;
  ComplexStateOp::A* a = new ComplexStateOp::A();
  op_impl->a_vec_ = {a};

  auto* clone = op->Clone();
  auto* clone_impl = clone->GetImplementation<ComplexStateOp>();

  EXPECT_EQ(clone->frequency_, 42u);
  EXPECT_EQ(clone->active_target_, kOpenCl);
  EXPECT_EQ(clone_impl->b_, true);
  EXPECT_NE(clone_impl->a_vec_[0], nullptr);
  // Since we do an explicit deep copy of the ComplexStateOp::a_vec_ vector, we
  // don't expect the same values here
  EXPECT_NE(clone_impl->a_vec_[0], a);

  delete a;
  delete op;
  delete clone_impl->a_vec_[0];
  delete clone;
}

TEST(SchedulerTest, MultipleSimulations) {
  Simulation* sim1 = new Simulation("sim1");
  Simulation* sim2 = new Simulation("sim2");

  Cell* cell = new Cell(10);
  Cell* cell2 = new Cell(10);

  sim1->Activate();
  sim1->GetResourceManager()->push_back(cell);
  auto* op1 = GET_OP("test_op");
  sim1->GetScheduler()->ScheduleOp(op1);
  sim1->Simulate(10);

  sim2->Activate();
  sim2->GetResourceManager()->push_back(cell2);
  auto* op2 = GET_OP("test_op");
  sim2->GetScheduler()->ScheduleOp(op2);

  auto* op1_impl = op1->GetImplementation<TestOp>();
  auto* op2_impl = op2->GetImplementation<TestOp>();

  EXPECT_EQ(10u, op1_impl->counter);
  EXPECT_EQ(0u, op2_impl->counter);

  sim2->Simulate(10);

  EXPECT_EQ(10u, op1_impl->counter);
  EXPECT_EQ(10u, op2_impl->counter);

  sim1->Activate();
  sim1->GetScheduler()->UnscheduleOp(op1);
  sim1->Simulate(10);

  EXPECT_EQ(10u, op1_impl->counter);
  EXPECT_EQ(10u, op2_impl->counter);

  delete sim1;
  delete sim2;
}

TEST(SchedulerTest, DefaultOps) {
  Simulation sim(TEST_NAME);
  sim.GetResourceManager()->push_back(new Cell(10));
  auto* scheduler = sim.GetScheduler();
  sim.Simulate(1);

  auto def_ops = scheduler->GetListOfDefaultOps();

  for (auto& def_op : def_ops) {
    auto* op = scheduler->GetDefaultOp(def_op);
    EXPECT_EQ(op->name_, def_op);
  }

  // Try to get a non-default op
  auto* test_op = GET_OP("test_op");
  scheduler->ScheduleOp(test_op);
  auto* op = scheduler->GetDefaultOp("test_op");
  EXPECT_EQ(op, nullptr);
}

}  // namespace scheduler_test_internal
}  // namespace bdm
