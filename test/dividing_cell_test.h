#ifndef TEST_DIVIDING_CELL_TEST_H_
#define TEST_DIVIDING_CELL_TEST_H_

#include <array>

#include "base_simulation_test.h"

#include "param.h"

#include "cells/cell_factory.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace bdm {

using bdm::cells::CellFactory;
using bdm::simulation::ECM;
using bdm::simulation::Scheduler;

class DividingCellTest : public BaseSimulationTest {
 public:
  DividingCellTest() {
  }

  void simulate() override {
    Random::setSeed(1L);
    initPhysicalNodeMovementListener();
    std::array<double, 3> cellOrigin { 0.0, 3.0, 5.0 };
    auto cell = CellFactory::getCellInstance(cellOrigin);
    cell->setColorForAllPhysicalObjects(Param::kRed);
    auto soma = cell->getSomaElement();
    auto sphere = soma->getPhysicalSphere();

    auto scheduler = Scheduler::getInstance();

    for (int i = 0; i < 5000; i++) {
      scheduler->simulateOneStep();     // run the simulation
      if (sphere->getDiameter() < 20) {  // if small..
        sphere->changeVolume(350);      // .. increase volume
      } else {
        auto c2 = cell->divide();       // otherwise divide
        c2->setColorForAllPhysicalObjects(Param::kBlue);
      }
    }
  }

  std::string getTestName() const override {
    return "DividingCellTest";
  }
};

}  // namespace bdm

#endif  // TEST_DIVIDING_CELL_TEST_H_
