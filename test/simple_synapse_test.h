#ifndef TEST_SIMPLE_SYNAPSE_TEST_H_
#define TEST_SIMPLE_SYNAPSE_TEST_H_

#include <array>
#include <memory>

#include "base_simulation_test.h"
#include "param.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"
#include "synapse/physical_bouton.h"
#include "synapse/biological_bouton.h"
#include "synapse/physical_spine.h"
#include "synapse/biological_spine.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::LocalBiologyModule;
using physics::PhysicalNode;
using simulation::ECM;
using simulation::Scheduler;
using synapse::PhysicalBouton;
using synapse::BiologicalBouton;
using synapse::PhysicalSpine;
using synapse::BiologicalSpine;

class SimpleSynapseTest : public BaseSimulationTest {
 public:
  SimpleSynapseTest() {
  }

  void simulate() override {
    auto ecm = ECM::getInstance();
    Random::setSeed(1L);
    initPhysicalNodeMovementListener();

    int number_of_additional_nodes = 10;
    for (int i = 0; i < number_of_additional_nodes; i++) {
      auto coord = Random::nextNoise(500);
      physical_nodes_.push_back(ecm->createPhysicalNodeInstance(coord));
    }

    std::array<double, 3> up { 0.0, 0.0, 1.0 }, down { 0.0, 0.0, -1.0 };
    // 1) two cells : and excitatory (down) and an inhibitory one (up)
    auto excit = CellFactory::getCellInstance( { -2.5, 0, -30 });
    excit->setNeuroMLType(Cell::NeuroMLType::kExcitatatory);
    excit->setColorForAllPhysicalObjects(Param::kGreen);
    auto inhib = CellFactory::getCellInstance( { 2.5, 0, 30 });
    inhib->setNeuroMLType(Cell::NeuroMLType::kInhibitory);
    inhib->setColorForAllPhysicalObjects(Param::kRed);
    // 2) excitatory cell makes an axon, inhibitory cell makes a dendrite
    auto axon = excit->getSomaElement()->extendNewNeurite(up);
    axon->setAxon(true);
    auto axon_cyl = axon->getPhysicalCylinder();
    auto dendrite = inhib->getSomaElement()->extendNewNeurite(down);
    dendrite->setAxon(false);
    auto dendrite_cyl = dendrite->getPhysicalCylinder();
    //    elongate them
    auto scheduler = Scheduler::getInstance();
    while (axon->getLocation()[2] < dendrite->getLocation()[2]) {
      axon->elongateTerminalEnd(1 / Param::kSimulationTimeStep, up);
      dendrite->elongateTerminalEnd(1 / Param::kSimulationTimeStep, down);
      scheduler->simulateOneStep();
    }
    // 3) a bouton on the axon:
    //    create the physical part
    std::array<double, 3> global_coord { axon->getLocation()[2] + dendrite->getLocation()[2], 0, 0 };
    auto polar_axon_coord = axon_cyl->transformCoordinatesGlobalToPolar(global_coord);

    auto p_bouton = new PhysicalBouton(axon_cyl, { polar_axon_coord[0], polar_axon_coord[1] }, 3);
    axon_cyl->addExcrescence(PhysicalBouton::UPtr { p_bouton });
    //    create the biological part and set call backs
    auto b_bouton = BiologicalBouton::UPtr { new BiologicalBouton };
    b_bouton->setPhysicalBouton(p_bouton);
    p_bouton->setBiologicalBouton(std::move(b_bouton));

    // 4) a spine on the dendrite:
    //    create the physical part
    auto polar_dendrite_coord = dendrite_cyl->transformCoordinatesGlobalToPolar(global_coord);

    auto p_spine = new PhysicalSpine(dendrite_cyl, { polar_dendrite_coord[0], polar_dendrite_coord[1] }, 3);
    dendrite_cyl->addExcrescence(PhysicalSpine::UPtr { p_spine });
    //    create the biological part and set call backs
    auto b_spine = BiologicalSpine::UPtr { new BiologicalSpine() };
    b_spine->setPhysicalSpine(p_spine);
    p_spine->setBiologicalSpine(std::move(b_spine));

    // 5) synapse formation
    p_bouton->synapseWith(p_spine, true);
  }

  std::string getTestName() const override {
    return "SimpleSynapseTest";
  }
};

}  // namespace cx3d

#endif  // TEST_SIMPLE_SYNAPSE_TEST_H_
