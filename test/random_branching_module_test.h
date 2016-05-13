#ifndef TEST_RANDOM_BRANCHING_MODULE_TEST_H_
#define TEST_RANDOM_BRANCHING_MODULE_TEST_H_

#include <array>
#include <memory>

#include "base_simulation_test.h"
#include "param.h"
#include "java_util.h"
#include "matrix.h"
#include "sim_state_serialization_util.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::LocalBiologyModule;
using simulation::ECM;
using simulation::Scheduler;

class RandomBranchingModule : public LocalBiologyModule, public std::enable_shared_from_this<RandomBranchingModule> {
 public:
  RandomBranchingModule(const std::shared_ptr<JavaUtil2>& java)
      : java_ { java } {
  }
  RandomBranchingModule(const RandomBranchingModule&) = delete;
  RandomBranchingModule& operator=(const RandomBranchingModule&) = delete;

  void run() override {
    auto delta = java_->matrixRandomNoise3(0.1);
    direction_ = Matrix::add(direction_, delta);
    direction_ = Matrix::normalize(direction_);
    neurite_->getPhysical()->movePointMass(kSpeed, direction_);

    if (java_->getRandomDouble1() < kProbabilityToBifurcate) {
      auto nn = neurite_->bifurcate();
      nn[0]->getPhysical()->setColor(Param::kRed);
      nn[1]->getPhysical()->setColor(Param::kBlue);
      return;
    }
    if (java_->getRandomDouble1() < kProbabilityToBranch) {
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
    return UPtr { new RandomBranchingModule(java_) };
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

  std::shared_ptr<JavaUtil2> java_;
  NeuriteElement* neurite_;
  std::array<double, 3> direction_;
};

class RandomBranchingModuleTest : public BaseSimulationTest {
 public:
  RandomBranchingModuleTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm, const std::shared_ptr<JavaUtil2>& java) override {
    java->setRandomSeed1(1L);
    java->initPhysicalNodeMovementListener();

    for (int i = 0; i < 18; i++) {
      ecm->getPhysicalNodeInstance(java->matrixRandomNoise3(1000));
    }
    java->setRandomSeed1(7L);
    for (int i = 0; i < 1; i++) {
      auto c = CellFactory::getCellInstance(java->matrixRandomNoise3(40), ecm);
      c->setColorForAllPhysicalObjects(Param::kGray);
      auto neurite = c->getSomaElement()->extendNewNeurite( { 0, 0, 1 });
      neurite->getPhysicalCylinder()->setDiameter(2);
      neurite->addLocalBiologyModule(LocalBiologyModule::UPtr { new RandomBranchingModule(java) });
    }

    auto scheduler = Scheduler::getInstance(ecm);
    for (int i = 0; i < 500; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "RandomBranchingModuleTest";
  }
};

}  // namespace cx3d

#endif  // TEST_RANDOM_BRANCHING_MODULE_TEST_H_
