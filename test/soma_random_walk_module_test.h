#ifndef TEST_SOMA_RANDOM_WALK_MODULE_TEST_H_
#define TEST_SOMA_RANDOM_WALK_MODULE_TEST_H_

#include "base_simulation_test.h"
#include "param.h"
#include "matrix.h"
#include "java_util.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/local_biology_module.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::AbstractLocalBiologyModule;
using local_biology::LocalBiologyModule;
using simulation::ECM;
using simulation::Scheduler;

class SomaRandomWalkModule : public AbstractLocalBiologyModule {
 public:
  SomaRandomWalkModule(const std::shared_ptr<JavaUtil2>& java)
      : java_ { java } {
    direction_ = java_->matrixRandomNoise3(1.0);
  }
  SomaRandomWalkModule(const SomaRandomWalkModule&) = delete;
  SomaRandomWalkModule& operator=(const SomaRandomWalkModule&) = delete;

  std::shared_ptr<LocalBiologyModule> getCopy() const override {
    return std::shared_ptr<LocalBiologyModule> { new SomaRandomWalkModule(java_) };
  }

  bool isCopiedWhenSomaDivides() const override {
    return true;
  }

  void run() override {
    auto delta = java_->matrixRandomNoise3(0.1);
    direction_ = Matrix::add(direction_, delta);
    direction_ = Matrix::normalize(direction_);
    getCellElement()->move(kSpeed, direction_);
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{}");
    return sb;
  }

 private:
  static constexpr double kSpeed = 50;
  std::shared_ptr<JavaUtil2> java_;
  std::array<double, 3> direction_;
};

class SomaRandomWalkModuleTest : public BaseSimulationTest {
 public:
  SomaRandomWalkModuleTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm, const std::shared_ptr<JavaUtil2>& java) override {
    java->setRandomSeed1(1L);
    java->initPhysicalNodeMovementListener();

    for (int i = 0; i < 5; i++) {
      auto c = CellFactory::getCellInstance(java->matrixRandomNoise3(40), ecm);
      c->getSomaElement()->addLocalBiologyModule(
          std::shared_ptr<LocalBiologyModule> { new SomaRandomWalkModule(java) });
    }

    auto scheduler = Scheduler::getInstance(ecm);
    for (int i = 0; i < 1000; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "SomaRandomWalkModuleTest";
  }
};

}  // namespace cx3d

#endif  // TEST_SOMA_RANDOM_WALK_MODULE_TEST_H_
