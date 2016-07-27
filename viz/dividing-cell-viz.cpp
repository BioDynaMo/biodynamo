//
// Created by bogdan on 7/14/16.
//

#include "param.h"

#include "cells/cell_factory.h"
#include "physics/physical_node_movement_listener.h"

#include "simulation/scheduler.h"
#include "visualization/gui.h"
#include "../test/neurite_chemo_attraction_test.h"
#include <TApplication.h>
#include <physics/default_force.h>
#include <synapse/connection_maker.h>
#include <local_biology/local_biology_module.h>

using namespace bdm;
using bdm::cells::CellFactory;
using bdm::simulation::ECM;
using bdm::simulation::Scheduler;
using bdm::physics::PhysicalNodeMovementListener;
using bdm::physics::PhysicalObject;
using bdm::physics::DefaultForce;
using bdm::physics::PhysicalNode;
using bdm::physics::SpaceNode;
using bdm::local_biology::Cell;
using bdm::local_biology::CellElement;
using bdm::physics::Substance;
using bdm::synapse::ConnectionMaker;
using bdm::local_biology::LocalBiologyModule;

int main(int argc, char** argv){
  TApplication app("App", &argc, argv);

  ECM::getInstance()->clearAll();
  Cell::reset();
  CellElement::reset();
  PhysicalNode::reset();
  SpaceNode < PhysicalNode > ::reset();

  visualization::GUI::getInstance().Init();

  Random::setSeed(1L);
  PhysicalObject::setInterObjectForce(DefaultForce::UPtr(new DefaultForce()));

  std::array<double, 3> cellOrigin { 0.0, 3.0, 5.0 };
  auto cell = CellFactory::getCellInstance(cellOrigin);
  cell->setColorForAllPhysicalObjects(Param::kRed);
  auto soma = cell->getSomaElement();
  auto sphere = soma->getPhysicalSphere();

  auto scheduler = Scheduler::getInstance();

  for (int i = 0; i < 5000; i++) {
    scheduler->simulateOneStep();     // run the simulation
    if (sphere->getDiameter() < 20) {  // if small..
      sphere->changeVolume(350);      // .. increase volume
    } else {
      auto c2 = cell->divide();       // otherwise divide
      c2->setColorForAllPhysicalObjects(Param::kBlue);
    }
    visualization::GUI::getInstance().Update();
  }
  visualization::GUI::getInstance().Update();
  visualization::GUI::getInstance().CloseGeometry();

  app.Run();
}