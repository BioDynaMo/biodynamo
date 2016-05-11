#ifndef TEST_INTRACELLULAR_DIFFUSION_TEST_H_
#define TEST_INTRACELLULAR_DIFFUSION_TEST_H_

#include <iostream>
#include <array>
#include <memory>

#include "base_simulation_test.h"
#include "param.h"
#include "matrix.h"
#include "java_util.h"
#include "sim_state_serialization_util.h"
#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "physics/intracellular_substance.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::LocalBiologyModule;
using local_biology::AbstractLocalBiologyModule;
using physics::Substance;
using physics::IntracellularSubstance;
using simulation::ECM;
using simulation::Scheduler;

class InternalSecretor : public AbstractLocalBiologyModule {
 public:
  InternalSecretor() {
  }
  InternalSecretor(const InternalSecretor&) = delete;
  InternalSecretor& operator=(const InternalSecretor&) = delete;

  UPtr getCopy() const override {
    return UPtr { new InternalSecretor() };
  }

  void run() override {
    getCellElement()->getPhysical()->modifyIntracellularQuantity("tubulin", kSecretionRate);
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{");  // fixme should call super.simStateToJson

    SimStateSerializationUtil::keyValue(sb, "secretionRate", kSecretionRate);

    SimStateSerializationUtil::removeLastChar(sb);
    sb.append("}");
    return sb;
  }

 private:
  static constexpr double kSecretionRate = 60.0;
};

class GrowthCone : public AbstractLocalBiologyModule {
 public:
  GrowthCone(const std::shared_ptr<JavaUtil2>& java)
      : java_ { java },
        previous_dir_ { { 0.0, 0.0, 0.0 } } {
  }
  ~GrowthCone() {
  }
  GrowthCone(const GrowthCone&) = delete;
  GrowthCone& operator=(const GrowthCone&) = delete;

  /**
   *
   */
  UPtr getCopy() const override {
    return UPtr { new GrowthCone(java_) };
  }

  /**
   * initial direction is parallel to the cylinder axis
   * therefore we overwrite this method from the superclass:
   */
  void setCellElement(const std::shared_ptr<CellElement>& cell_element) override {
    AbstractLocalBiologyModule::setCellElement(cell_element);
    previous_dir_ = cell_element->getPhysical()->getAxis();
  }

  bool isCopiedWhenNeuriteBranches() const override {
    return true;
  }

  bool isDeletedAfterNeuriteHasBifurcated() const override {
    return true;
  }

  void run() override {
    // getting the concentration and defining the speed
    auto cyl = getCellElement()->getPhysical();
    double concentration = cyl->getIntracellularConcentration("tubulin");
    double speed = concentration * kSpeedFactor;
    if (speed > 100) {  // can't be faster than 100
      speed = 100;
    }
    // movement and consumption
    auto noise = java_->matrixRandomNoise3(0.1);
    auto direction = Matrix::add(previous_dir_, noise);
    previous_dir_ = Matrix::normalize(direction);
    cyl->movePointMass(speed, direction);
    cyl->modifyIntracellularQuantity("tubulin", -concentration * kConsumptionFactor);
    // test for bifurcation
    if (java_->getRandomDouble1() < kBifurcationProbability) {
      std::static_pointer_cast<NeuriteElement>(getCellElement())->bifurcate();
    }
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    AbstractLocalBiologyModule::simStateToJson(sb);

    SimStateSerializationUtil::keyValue(sb, "speedFactor", kSpeedFactor);
    SimStateSerializationUtil::keyValue(sb, "consumptionFactor", kConsumptionFactor);
    SimStateSerializationUtil::keyValue(sb, "bifurcationProba", kBifurcationProbability);
    SimStateSerializationUtil::keyValue(sb, "previousDir", previous_dir_);

    SimStateSerializationUtil::removeLastChar(sb);
    sb.append("}");
    return sb;
  }

 private:
  static constexpr double kSpeedFactor = 5000;
  static constexpr double kConsumptionFactor = 100;
  static constexpr double kBifurcationProbability = 0.003;

  std::shared_ptr<JavaUtil2> java_;
  std::array<double, 3> previous_dir_;
};

class IntracellularDiffusionTest : public BaseSimulationTest {
 public:
  IntracellularDiffusionTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm, const std::shared_ptr<JavaUtil2>& java) override {
    java->setRandomSeed1(1L);
    java->initPhysicalNodeMovementListener();

    for (int i = 0; i < 18; i++) {
      ecm->getPhysicalNodeInstance(java->matrixRandomNoise3(500));
    }

    // defining the templates for the intracellular substance
    double D = 1000;  // diffusion cst
    double d = 0.01;  // degradation cst
    auto tubulin = IntracellularSubstance::create("tubulin", D, d);
    tubulin->setVolumeDependant(false);
    ecm->addNewIntracellularSubstanceTemplate(tubulin);
    // getting a cell
    auto c = CellFactory::getCellInstance( { 0.0, 0.0, 0.0 }, ecm);
    c->setColorForAllPhysicalObjects(Param::kRed);
    // insert production module
    auto soma = c->getSomaElement();
    soma->addLocalBiologyModule(LocalBiologyModule::UPtr { new InternalSecretor() });
    //insert growth cone module
    auto ne = c->getSomaElement()->extendNewNeurite( { 0, 0, 1 });
    ne->getPhysical()->setDiameter(1.0);
    ne->addLocalBiologyModule(LocalBiologyModule::UPtr { new GrowthCone(java) });
    // run, Forrest, run..
    auto scheduler = Scheduler::getInstance(ecm);
    for (int i = 0; i < 2001; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "IntracellularDiffusionTest";
  }
};

}  // namespace cx3d

#endif  // TEST_INTRACELLULAR_DIFFUSION_TEST_H_

