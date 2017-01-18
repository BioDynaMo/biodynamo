#ifndef TEST_SOMA_CLUSTERING_TEST_H_
#define TEST_SOMA_CLUSTERING_TEST_H_

#include <array>
#include <memory>
#include <string>

#include "base_simulation_test.h"
#include "param.h"
#include "matrix.h"

#include "sim_state_serialization_util.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "physics/substance.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace bdm {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::AbstractLocalBiologyModule;
using local_biology::LocalBiologyModule;
using physics::PhysicalNode;
using physics::Substance;
using simulation::ECM;
using simulation::Scheduler;

class SomaClustering : public AbstractLocalBiologyModule {
 public:
  SomaClustering(const std::string& substance_id)
      : substance_id_ { substance_id } {
  }
  SomaClustering(const SomaClustering&) = delete;
  SomaClustering& operator=(const SomaClustering&) = delete;

  UPtr getCopy() const override {
    return UPtr { new SomaClustering(substance_id_) };
  }

  void run() override {
    auto physical = getCellElement()->getPhysical();
    // move
    auto grad = physical->getExtracellularGradient(substance_id_);
    physical->movePointMass(kSpeed, Matrix::normalize(grad));
    // secrete
    physical->modifyExtracellularQuantity(substance_id_, 1000);
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    AbstractLocalBiologyModule::simStateToJson(sb);
    SimStateSerializationUtil::keyValue(sb, "substanceID", substance_id_);
    SimStateSerializationUtil::removeLastChar(sb);
    sb.append("}");
    return sb;
  }

 private:
  static constexpr double kSpeed = 100;

  std::string substance_id_;
};

class SomaClusteringTest : public BaseSimulationTest {
 public:
  SomaClusteringTest() {
  }

  void simulate() override {
    auto ecm = ECM::getInstance();
    Random::setSeed(1L);
    initPhysicalNodeMovementListener();

    auto yellow_substance = Substance::UPtr(new Substance("Yellow", 1000, 0.01));
    auto violet_substance = Substance::UPtr(new Substance("Violet", 1000, 0.01));
    ecm->addNewSubstanceTemplate(std::move(yellow_substance));
    ecm->addNewSubstanceTemplate(std::move(violet_substance));
    for (int i = 0; i < 400; i++) {
      physical_nodes_.push_back(ecm->createPhysicalNodeInstance(Random::nextNoise(700)));
    }
    for (int i = 0; i < 60; i++) {
      auto c = CellFactory::getCellInstance(Random::nextNoise(50));
      c->getSomaElement()->addLocalBiologyModule(LocalBiologyModule::UPtr { new SomaClustering("Yellow") });
      c->setColorForAllPhysicalObjects(Param::kYellowSolid);
    }
    for (int i = 0; i < 60; i++) {
      auto c = CellFactory::getCellInstance(Random::nextNoise(50));
      c->getSomaElement()->addLocalBiologyModule(LocalBiologyModule::UPtr { new SomaClustering("Violet") });
      c->setColorForAllPhysicalObjects(Param::kVioletSolid);
    }
    auto scheduler = Scheduler::getInstance();
    for (int i = 0; i < 1000; i++) {
      scheduler->simulateOneStep();
      if (i == 500 || i == 501 || i == 510) {
        checkpoint();
      }
    }
  }

  std::string getTestName() const override {
    return "SomaClusteringTest";
  }
};

}  // namespace bdm

#endif  // TEST_SOMA_CLUSTERING_TEST_H_
