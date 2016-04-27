#ifndef TEST_DIVIDING_CELL_TEST_H_
#define TEST_DIVIDING_CELL_TEST_H_

#include <array>

#include "param.h"
#include "java_util.h"

#include "cells/cell_factory.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cx3d::cells::CellFactory;
using cx3d::simulation::ECM;
using cx3d::simulation::Scheduler;

class DividingCellTest {
 public:
  DividingCellTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm) {
    std::array<double, 3> cellOrigin { 0.0, 3.0, 5.0 };
    auto cell = CellFactory::getCellInstance(cellOrigin, ecm);
    cell->setColorForAllPhysicalObjects(Param::kRed);
    auto soma = cell->getSomaElement();
    auto sphere = soma->getPhysicalSphere();

    auto scheduler = Scheduler::getInstance(ecm);

    for (int i = 0; i < 5000; i++) {
      scheduler->simulateOneStep();     // run the simulation
      if (sphere->getDiameter() < 20) { // if small..
        sphere->changeVolume(350);      // .. increase volume
      } else {
        auto c2 = cell->divide();       // otherwise divide
        c2->setColorForAllPhysicalObjects(Param::kBlue);
      }
    }
  }
};

}

#endif  // TEST_DIVIDING_CELL_TEST_H_
