#ifndef TEST_DIVIDING_MODULE_TEST_H_
#define TEST_DIVIDING_MODULE_TEST_H_

#include <array>

#include "param.h"
#include "java_util.h"

#include "cells/cell.h"
#include "cells/cell_module.h"
#include "cells/cell_factory.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cx3d::cells::Cell;
using cx3d::cells::CellModule;
using cx3d::cells::CellFactory;
using cx3d::simulation::ECM;
using cx3d::simulation::Scheduler;

class DividingModule : public CellModule {
 public:
  DividingModule() {
  }
  DividingModule(const DividingModule&) = delete;
  DividingModule& operator=(const DividingModule&) = delete;

  std::shared_ptr<Cell> getCell() const override {
    return cell_;
  }

  void setCell(const std::shared_ptr<Cell>& cell) override {
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

  std::shared_ptr<CellModule> getCopy() const override {
    return std::shared_ptr<CellModule> { new DividingModule() };
  }

  bool isCopiedWhenCellDivides() const override {
    return true;
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{}");
    return sb;
  }

 private:
  std::shared_ptr<Cell> cell_;
};

class DividingModuleTest {
 public:
  DividingModuleTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm) {
    auto c = CellFactory::getCellInstance( { 0.0, 0.0, 0.0 }, ecm);
    c->addCellModule(std::shared_ptr<DividingModule> { new DividingModule() });

    auto scheduler = Scheduler::getInstance(ecm);
    scheduler->simulateOneStep();
    scheduler->simulateOneStep();
    for (int i = 0; i < 5000; i++) {
      scheduler->simulateOneStep();
    }
  }
};

}

#endif  // TEST_DIVIDING_MODULE_TEST_H_
