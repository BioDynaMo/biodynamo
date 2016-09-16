#include <TApplication.h>
#include "cells/cell_factory.h"
#include "physics/default_force.h"
#include "simulation/scheduler.h"
#include "synapse/connection_maker.h"
#include "visualization/gui.h"

using bdm::cells::CellFactory;
using bdm::simulation::ECM;
using bdm::simulation::Scheduler;
using bdm::physics::PhysicalObject;
using bdm::physics::DefaultForce;
using bdm::physics::PhysicalNode;
using bdm::local_biology::Cell;
using bdm::local_biology::CellElement;
using bdm::physics::Substance;
using bdm::synapse::ConnectionMaker;
using bdm::local_biology::LocalBiologyModule;
using bdm::visualization::Gui;

int main(int argc, char** argv){
  TApplication app("App", &argc, argv);

  Gui::getInstance().Init();

  bdm::Random::setSeed(1L);
  PhysicalObject::setInterObjectForce(DefaultForce::UPtr(new DefaultForce()));

  std::array<double, 3> cell_origin { 0.0, 3.0, 5.0 };
  auto cell = CellFactory::getCellInstance(cell_origin);
  cell->setColorForAllPhysicalObjects(bdm::Param::kRed);
  auto soma = cell->getSomaElement();
  auto sphere = soma->getPhysicalSphere();

  auto scheduler = Scheduler::getInstance();

  for (int i = 0; i < 5000; i++) {
    scheduler->simulateOneStep();     // run the simulation
    if (sphere->getDiameter() < 20) {  // if small..
      sphere->changeVolume(350);      // .. increase volume
    } else {
      auto c2 = cell->divide();       // otherwise divide
      c2->setColorForAllPhysicalObjects(bdm::Param::kBlue);
    }
    Gui::getInstance().Update();
  }
  Gui::getInstance().Update();
  Gui::getInstance().CloseGeometry();

  app.Run();
}
