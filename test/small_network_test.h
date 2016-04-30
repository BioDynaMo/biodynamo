#ifndef TEST_SMALL_NETWORK_TEST_H_
#define TEST_SMALL_NETWORK_TEST_H_

#include <array>
#include <memory>

#include "base_simulation_test.h"
#include "neurite_chemo_attraction_test.h"
#include "param.h"
#include "java_util.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "physics/substance.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"
#include "synapse/physical_bouton.h"
#include "synapse/biological_bouton.h"
#include "synapse/physical_spine.h"
#include "synapse/biological_spine.h"
#include "synapse/connection_maker.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::LocalBiologyModule;
using simulation::ECM;
using simulation::Scheduler;
using physics::Substance;
using synapse::PhysicalBouton;
using synapse::BiologicalBouton;
using synapse::PhysicalSpine;
using synapse::BiologicalSpine;
using synapse::ConnectionMaker;

class SmallNetworkTest : public BaseSimulationTest {
 public:
  SmallNetworkTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm, const std::shared_ptr<JavaUtil2>& java) override {
    java->setRandomSeed1(1L);
    java->initPhysicalNodeMovementListener();

    auto L1 = Substance::create("L1", Color(0xFFFF0000));  // Color is red
    ecm->addArtificialGaussianConcentrationZ(L1, 1.0, 400.0, 60.0);

    int number_of_nodes = 10;
    for (int i = 0; i < number_of_nodes; i++) {
      auto coord = java->matrixRandomNoise3(500);
      ecm->getPhysicalNodeInstance(coord);
    }

    for (int i = 0; i < 8; i++) {
      std::shared_ptr<Cell> c;
      double rand_1 = java->getRandomDouble1();
      double rand_2 = java->getRandomDouble1();
      if (i < 4) {
        c = CellFactory::getCellInstance( { -20 + 40 * rand_1, -20 + 40 * rand_2, 0.0 }, ecm);
        c->setNeuroMLType(Cell::NeuroMLType::kExcitatatory);
        c->setColorForAllPhysicalObjects(Param::kViolet);
      } else {
        c = CellFactory::getCellInstance( { -20 + 40 * rand_1, -20 + 40 * rand_2, 200.0 }, ecm);
        c->setNeuroMLType(Cell::NeuroMLType::kInhibitory);
        c->setColorForAllPhysicalObjects(Color(0xB38200AC));  // darker Param::kViolet
      }
      auto axon = c->getSomaElement()->extendNewNeurite();
      axon->setAxon(true);
      axon->getPhysicalCylinder()->setDiameter(0.5);
      axon->addLocalBiologyModule(std::shared_ptr<LocalBiologyModule> { new NeuriteChemoAttraction("L1", 0.02, java) });

      if (i < 4) {
        axon->getPhysicalCylinder()->setColor(Param::kYellow);
      } else {
        axon->getPhysicalCylinder()->setColor(Color(0xB3B29415));  // darker Param::Yellow
      }

      auto dendrite = c->getSomaElement()->extendNewNeurite();
      dendrite->setAxon(false);
      dendrite->getPhysicalCylinder()->setDiameter(1.5);
      dendrite->addLocalBiologyModule(
          std::shared_ptr<LocalBiologyModule> { new NeuriteChemoAttraction("L1", 0.02, java) });
    }
    auto scheduler = Scheduler::getInstance(ecm);
    while (ecm->getECMtime() < 6) {
      scheduler->simulateOneStep();
    }
    ConnectionMaker::extendExcressencesAndSynapseOnEveryNeuriteElement(ecm);
  }

  std::string getTestName() const override {
    return "SmallNetworkTest";
  }
};

}  // namespace cx3d

#endif  // TEST_SMALL_NETWORK_TEST_H_
