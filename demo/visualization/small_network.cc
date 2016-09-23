#include <chrono>
#include <TApplication.h>
#include "../test/neurite_chemo_attraction_test.h"
#include "physics/default_force.h"
#include "physics/physical_node_movement_listener.h"
#include "synapse/connection_maker.h"
#include "visualization/gui.h"

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
using bdm::visualization::Gui;

int main(int argc, char **argv) {
  TApplication app("App", &argc, argv);

  auto ecm = ECM::getInstance();
  Gui::getInstance().Init();

  PhysicalObject::setInterObjectForce(DefaultForce::UPtr(new DefaultForce()));

  bdm::Random::setSeed(1L);

  auto L1 =
      Substance::UPtr(new Substance("L1", bdm::Color(0xFFFF0000)));  // Color is red
  ecm->addArtificialGaussianConcentrationZ(L1.get(), 1.0, 400.0, 60.0);

  int number_of_nodes = 10;
  for (int i = 0; i < number_of_nodes; i++) {
    auto coord = bdm::Random::nextNoise(500);
    // commented, because test fails with segfault during runtime
    // ecm->createPhysicalNodeInstance(coord);
  }

  for (int i = 0; i < 8; i++) {
    Cell *c;
    double rand_1 = bdm::Random::nextDouble();
    double rand_2 = bdm::Random::nextDouble();
    if (i < 4) {
      c = CellFactory::getCellInstance(
          {-20 + 40 * rand_1, -20 + 40 * rand_2, 0.0});
      c->setNeuroMLType(Cell::NeuroMLType::kExcitatatory);
      c->setColorForAllPhysicalObjects(bdm::Param::kViolet);
    } else {
      c = CellFactory::getCellInstance(
          {-20 + 40 * rand_1, -20 + 40 * rand_2, 200.0});
      c->setNeuroMLType(Cell::NeuroMLType::kInhibitory);
      // darker Param::kViolet
      c->setColorForAllPhysicalObjects(bdm::Color(0xB38200AC));
    }
    auto axon = c->getSomaElement()->extendNewNeurite();
    axon->setAxon(true);
    axon->getPhysicalCylinder()->setDiameter(0.5);
    axon->addLocalBiologyModule(
        LocalBiologyModule::UPtr{new bdm::NeuriteChemoAttraction("L1", 0.02)});

    if (i < 4) {
      axon->getPhysicalCylinder()->setColor(bdm::Param::kYellow);
    } else {
      // darker Param::Yellow
      axon->getPhysicalCylinder()->setColor(bdm::Color(0xB3B29415));
    }

    auto dendrite = c->getSomaElement()->extendNewNeurite();
    dendrite->setAxon(false);
    dendrite->getPhysicalCylinder()->setDiameter(1.5);
    dendrite->addLocalBiologyModule(
        LocalBiologyModule::UPtr{new bdm::NeuriteChemoAttraction("L1", 0.02)});
  }

  auto scheduler = Scheduler::getInstance();
  auto max_time = 6;
  auto begin = std::chrono::steady_clock::now();

  while (ecm->getECMtime() < max_time) {
    auto middle = std::chrono::steady_clock::now();

    scheduler->simulateOneStep();
    Gui::getInstance().Update();
  }

  int objects =
      ecm->getPhysicalSphereListSize() + ecm->getPhysicalCylinderListSize();
  printf("[Info] Total objects in simulation: %d\n", objects);

  ConnectionMaker::extendExcressencesAndSynapseOnEveryNeuriteElement();

  auto begin_upd = std::chrono::steady_clock::now();

  // don't forget to close geometry in the end
  bdm::visualization::Gui::getInstance().CloseGeometry();

  auto end_upd = std::chrono::steady_clock::now();
  double viz_time = std::chrono::duration_cast<std::chrono::microseconds>(
                       end_upd - begin_upd).count() / 1e3;

  printf("[Info] Total visualization time for one frame: %2.1f ms\n", viz_time);

  app.Run();
}
