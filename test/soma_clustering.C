#include <array>
#include <memory>
#include <string>

#include <TROOT.h>

#include "soma_clustering_test.h"
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
#include "physics/default_force.h"
#include "physics/physical_node_movement_listener.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"


using namespace bdm;

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::AbstractLocalBiologyModule;
using local_biology::LocalBiologyModule;
using physics::PhysicalNode;
using physics::Substance;
using physics::DefaultForce;
using physics::PhysicalObject;
using physics::PhysicalNodeMovementListener;
using simulation::ECM;
using simulation::Scheduler;
using spatial_organization::SpaceNode;
using synapse::Excrescence;


std::vector<physics::PhysicalNode::UPtr> physical_nodes;

void simulate()
{
   auto ecm = ECM::getInstance();
   Random::setSeed(1L);
   PhysicalNodeMovementListener::setMovementOperationId((int)(10000 * Random::nextDouble()));

   auto yellow_substance = Substance::UPtr(new Substance("Yellow", 1000, 0.01));
   auto violet_substance = Substance::UPtr(new Substance("Violet", 1000, 0.01));
   ecm->addNewSubstanceTemplate(std::move(yellow_substance));
   ecm->addNewSubstanceTemplate(std::move(violet_substance));
   for (int i = 0; i < 400; i++) {
      physical_nodes.push_back(ecm->createPhysicalNodeInstance(Random::nextNoise(700)));
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
   }
}

int soma_clustering()
{
   // Run the soma clsutering test

   // setup
   ECM::getInstance()->clearAll();
   Cell::reset();
   CellElement::reset();
   PhysicalNode::reset();
   SpaceNode < PhysicalNode > ::reset();

   Random::setSeed(1L);

   PhysicalObject::setInterObjectForce(DefaultForce::UPtr(new DefaultForce()));

   // run simulation
   gROOT->Time();
   simulate();

   return 0;
}
