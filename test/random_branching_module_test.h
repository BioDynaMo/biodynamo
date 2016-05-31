#ifndef TEST_RANDOM_BRANCHING_MODULE_TEST_H_
#define TEST_RANDOM_BRANCHING_MODULE_TEST_H_

#include <array>
#include <memory>

#include "base_simulation_test.h"
#include "param.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace bdm {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::LocalBiologyModule;
using physics::PhysicalNode;
using simulation::ECM;
using simulation::Scheduler;

class RandomBranchingModule : public LocalBiologyModule, public std::enable_shared_from_this<RandomBranchingModule> {
 public:
  RandomBranchingModule() {
  }
  RandomBranchingModule(const RandomBranchingModule&) = delete;
  RandomBranchingModule& operator=(const RandomBranchingModule&) = delete;

  void run() override {
    auto delta = Random::nextNoise(0.1);
    direction_ = Matrix::add(direction_, delta);
    direction_ = Matrix::normalize(direction_);
    neurite_->getPhysical()->movePointMass(kSpeed, direction_);

    if (Random::nextDouble() < kProbabilityToBifurcate) {
      auto nn = neurite_->bifurcate();
      nn[0]->getPhysical()->setColor(Param::kRed);
      nn[1]->getPhysical()->setColor(Param::kBlue);
      return;
    }
    if (Random::nextDouble() < kProbabilityToBranch) {
      auto n = neurite_->branch();
      n->getPhysical()->setColor(Param::kViolet);
      return;
    }
  }

  CellElement* getCellElement() const override {
    return neurite_;
  }

  void setCellElement(CellElement* cell_element) override {
    if (cell_element->isANeuriteElement()) {
      neurite_ = static_cast<NeuriteElement*>(cell_element);
      direction_ = neurite_->getPhysicalCylinder()->getAxis();
    } else {
      // Sorry, I only work with neurite elements
      cell_element->removeLocalBiologyModule(this);
    }
  }

  UPtr getCopy() const override {
    return UPtr { new RandomBranchingModule() };
  }

  bool isCopiedWhenNeuriteBranches() const override {
    return true;
  }

  bool isCopiedWhenSomaDivides() const override {
    return false;  // this method should never be called
  }

  bool isCopiedWhenNeuriteElongates() const override {
    return false;  // only in growth cones
  }

  bool isCopiedWhenNeuriteExtendsFromSoma() const override {
    return false;  // this method should never be called
  }

  bool isDeletedAfterNeuriteHasBifurcated() const override {
    return true;  // Important because of bifurcation
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{");

    SimStateSerializationUtil::keyValue(sb, "direction", direction_);

    SimStateSerializationUtil::removeLastChar(sb);
    sb.append("}");
    return sb;
  }

 private:
  static constexpr double kSpeed = 100;
  static constexpr double kProbabilityToBifurcate = 0.005;
  static constexpr double kProbabilityToBranch = 0.005;

  NeuriteElement* neurite_;
  std::array<double, 3> direction_;
};

class RandomBranchingModuleTest : public BaseSimulationTest {
 public:
  RandomBranchingModuleTest() {
  }

  void simulate() override {
    auto ecm = ECM::getInstance();
    Random::setSeed(1L);
    initPhysicalNodeMovementListener();

    for (int i = 0; i < 18; i++) {
      physical_nodes_.push_back(ecm->createPhysicalNodeInstance(Random::nextNoise(1000)));
    }
    Random::setSeed(7L);
    for (int i = 0; i < 1; i++) {
      auto c = CellFactory::getCellInstance(Random::nextNoise(40));
      c->setColorForAllPhysicalObjects(Param::kGray);
      auto neurite = c->getSomaElement()->extendNewNeurite( { 0, 0, 1 });
      neurite->getPhysicalCylinder()->setDiameter(2);
      neurite->addLocalBiologyModule(LocalBiologyModule::UPtr { new RandomBranchingModule() });
    }

    auto scheduler = Scheduler::getInstance();
    for (int i = 0; i < 500; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "RandomBranchingModuleTest";
  }
};

}  // namespace bdm

#endif  // TEST_RANDOM_BRANCHING_MODULE_TEST_H_
