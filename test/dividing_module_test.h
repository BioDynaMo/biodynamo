#ifndef TEST_DIVIDING_MODULE_TEST_H_
#define TEST_DIVIDING_MODULE_TEST_H_

#include <array>

#include "base_simulation_test.h"
#include "param.h"

#include "cells/cell.h"
#include "cells/cell_module.h"
#include "cells/cell_factory.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace bdm {

using bdm::cells::Cell;
using bdm::cells::CellModule;
using bdm::cells::CellFactory;
using bdm::simulation::ECM;
using bdm::simulation::Scheduler;

class DividingModule : public CellModule {
 public:
  DividingModule() {
  }
  DividingModule(const DividingModule&) = delete;
  DividingModule& operator=(const DividingModule&) = delete;

  Cell* getCell() const override {
    return cell_;
  }

  void setCell(Cell* cell) override {
    cell_ = cell;
  }

  void run() override {
    auto sphere = cell_->getSomaElement()->getPhysicalSphere();
    if (sphere->getDiameter() > 20) {
      cell_->divide();
    } else {
      sphere->changeVolume(300);
    }
  }

  CellModule::UPtr getCopy() const override {
    return CellModule::UPtr { new DividingModule() };
  }

  bool isCopiedWhenCellDivides() const override {
    return true;
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{}");
    return sb;
  }

 private:
  Cell* cell_ = nullptr;
};

class DividingModuleTest : public BaseSimulationTest {
 public:
  DividingModuleTest() {
  }

  void simulate() override {
    Random::setSeed(2L);
    initPhysicalNodeMovementListener();

    auto c = CellFactory::getCellInstance( { 0.0, 0.0, 0.0 });
    c->addCellModule(CellModule::UPtr { new DividingModule() });

    auto scheduler = Scheduler::getInstance();
    scheduler->simulateOneStep();
    scheduler->simulateOneStep();
    for (int i = 0; i < 5000; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "DividingModuleTest";
  }
};

}  // namespace bdm

#endif  // TEST_DIVIDING_MODULE_TEST_H_
