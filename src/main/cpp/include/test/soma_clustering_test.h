#ifndef TEST_SOMA_CLUSTERING_TEST_H_
#define TEST_SOMA_CLUSTERING_TEST_H_

#include <array>
#include <memory>
#include <string>

#include "param.h"
#include "matrix.h"
#include "java_util.h"
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

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::AbstractLocalBiologyModule;
using local_biology::LocalBiologyModule;
using physics::Substance;
using simulation::ECM;
using simulation::Scheduler;

class SomaClustering : public AbstractLocalBiologyModule {
 public:
  SomaClustering(const std::string& substance_id, const std::shared_ptr<JavaUtil2>& java)
      : java_ { java },
        substance_id_ { substance_id } {
  }
  SomaClustering(const SomaClustering&) = delete;
  SomaClustering& operator=(const SomaClustering&) = delete;

  std::shared_ptr<LocalBiologyModule> getCopy() const override {
    return std::shared_ptr<LocalBiologyModule> { new SomaClustering(substance_id_, java_) };
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

  std::shared_ptr<JavaUtil2> java_;
  std::string substance_id_;
};

class SomaClusteringTest {
 public:
  SomaClusteringTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm, const std::shared_ptr<JavaUtil2>& java) {
    auto yellow_substance = Substance::create("Yellow", 1000, 0.01);
    auto violet_substance = Substance::create("Violet", 1000, 0.01);
    ecm->addNewSubstanceTemplate(yellow_substance);
    ecm->addNewSubstanceTemplate(violet_substance);
    for (int i = 0; i < 400; i++) {
      ecm->getPhysicalNodeInstance(java->matrixRandomNoise3(700));
    }
    for (int i = 0; i < 60; i++) {
      auto c = CellFactory::getCellInstance(java->matrixRandomNoise3(50), ecm);
      c->getSomaElement()->addLocalBiologyModule(std::shared_ptr<LocalBiologyModule> { new SomaClustering("Yellow", java) });
      c->setColorForAllPhysicalObjects(Param::kYellowSolid);
    }
    for (int i = 0; i < 60; i++) {
      auto c = CellFactory::getCellInstance(java->matrixRandomNoise3(50), ecm);
      c->getSomaElement()->addLocalBiologyModule(std::shared_ptr<LocalBiologyModule> { new SomaClustering("Violet", java) });
      c->setColorForAllPhysicalObjects(Param::kVioletSolid);
    }
    auto scheduler = Scheduler::getInstance(ecm);
    for (int i = 0; i < 1000; i++) {
      scheduler->simulateOneStep();
    }
  }
};

}  // namespace cx3d

#endif  // TEST_SOMA_CLUSTERING_TEST_H_
