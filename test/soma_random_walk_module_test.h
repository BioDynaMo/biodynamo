#ifndef TEST_SOMA_RANDOM_WALK_MODULE_TEST_H_
#define TEST_SOMA_RANDOM_WALK_MODULE_TEST_H_

#include "base_simulation_test.h"
#include "param.h"
#include "matrix.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/local_biology_module.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace bdm {

using cells::Cell;
using cells::CellFactory;
using local_biology::AbstractLocalBiologyModule;
using local_biology::LocalBiologyModule;
using simulation::ECM;
using simulation::Scheduler;

class SomaRandomWalkModule : public AbstractLocalBiologyModule {
 public:
  SomaRandomWalkModule() {
    direction_ = Random::nextNoise(1.0);
  }
  SomaRandomWalkModule(const SomaRandomWalkModule&) = delete;
  SomaRandomWalkModule& operator=(const SomaRandomWalkModule&) = delete;

  UPtr getCopy() const override {
    return UPtr { new SomaRandomWalkModule() };
  }

  bool isCopiedWhenSomaDivides() const override {
    return true;
  }

  void run() override {
    auto delta = Random::nextNoise(0.1);
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
  std::array<double, 3> direction_;
};

class SomaRandomWalkModuleTest : public BaseSimulationTest {
 public:
  SomaRandomWalkModuleTest() {
  }

  void simulate() override {
    Random::setSeed(1L);
    initPhysicalNodeMovementListener();

    for (int i = 0; i < 5; i++) {
      auto c = CellFactory::getCellInstance(Random::nextNoise(40));
      c->getSomaElement()->addLocalBiologyModule(SomaRandomWalkModule::UPtr { new SomaRandomWalkModule() });
    }

    auto scheduler = Scheduler::getInstance();
    for (int i = 0; i < 4612079; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "SomaRandomWalkModuleTest";
  }
};

}  // namespace bdm

#endif  // TEST_SOMA_RANDOM_WALK_MODULE_TEST_H_
