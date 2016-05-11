#ifndef TEST_SIMPLE_SYNAPSE_TEST_H_
#define TEST_SIMPLE_SYNAPSE_TEST_H_

#include <array>
#include <memory>

#include "param.h"
#include "java_util.h"

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
using simulation::ECM;
using simulation::Scheduler;
using synapse::PhysicalBouton;
using synapse::BiologicalBouton;
using synapse::PhysicalSpine;
using synapse::BiologicalSpine;

class SimpleSynapseTest {
 public:
  SimpleSynapseTest() {
  }

  void simulate(const std::shared_ptr<ECM>& ecm, const std::shared_ptr<JavaUtil2>& java) {
    java->setRandomSeed1(1L);
    java->initPhysicalNodeMovementListener();

    int number_of_additional_nodes = 10;
    for (int i = 0; i < number_of_additional_nodes; i++) {
      auto coord = java->matrixRandomNoise3(500);
      ecm->getPhysicalNodeInstance(coord);
    }

    std::array<double, 3> up { 0.0, 0.0, 1.0 }, down { 0.0, 0.0, -1.0 };
    // 1) two cells : and excitatory (down) and an inhibitory one (up)
    auto excit = CellFactory::getCellInstance( { -2.5, 0, -30 }, ecm);
    excit->setNeuroMLType(Cell::NeuroMLType::kExcitatatory);
    excit->setColorForAllPhysicalObjects(Param::kGreen);
    auto inhib = CellFactory::getCellInstance( { 2.5, 0, 30 }, ecm);
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
    auto scheduler = Scheduler::getInstance(ecm);
    while (axon->getLocation()[2] < dendrite->getLocation()[2]) {
      axon->elongateTerminalEnd(1 / Param::kSimulationTimeStep, up);
      dendrite->elongateTerminalEnd(1 / Param::kSimulationTimeStep, down);
      scheduler->simulateOneStep();
    }
    // 3) a bouton on the axon:
    //    create the physical part
    std::array<double, 3> global_coord { axon->getLocation()[2] + dendrite->getLocation()[2], 0, 0 };
    auto polar_axon_coord = axon_cyl->transformCoordinatesGlobalToPolar(global_coord);

    auto p_bouton = PhysicalBouton::create(axon_cyl, { polar_axon_coord[0], polar_axon_coord[1] }, 3);
    axon_cyl->addExcrescence(p_bouton);
    //    create the biological part and set call backs
    auto b_bouton = BiologicalBouton::create();
    p_bouton->setBiologicalBouton(b_bouton);
    b_bouton->setPhysicalBouton(p_bouton);

    // 4) a spine on the dendrite:
    //    create the physical part
    auto polar_dendrite_coord = dendrite_cyl->transformCoordinatesGlobalToPolar(global_coord);

    auto p_spine = PhysicalSpine::create(dendrite_cyl, { polar_dendrite_coord[0], polar_dendrite_coord[1] }, 3);
    dendrite_cyl->addExcrescence(p_spine);
    //    create the biological part and set call backs
    auto b_spine = BiologicalSpine::create();
    p_spine->setBiologicalSpine(b_spine);
    b_spine->setPhysicalSpine(p_spine);

    // 5) synapse formation
    p_bouton->synapseWith(p_spine, true);
  }
};

}  // namespace cx3d

#endif  // TEST_SIMPLE_SYNAPSE_TEST_H_
