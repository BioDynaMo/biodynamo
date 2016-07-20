//
// Created by bogdan on 7/14/16.
//
#include <chrono>
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

int main(int argc, char **argv) {
  TApplication app("App", &argc, argv);

  ECM::getInstance()->clearAll();
  Cell::reset();
  CellElement::reset();
  PhysicalNode::reset();
  SpaceNode<PhysicalNode>::reset();

  auto ecm = ECM::getInstance();
  visualization::GUI::getInstance().Init();

  PhysicalObject::setInterObjectForce(DefaultForce::UPtr(new DefaultForce()));

  Random::setSeed(1L);

  auto L1 =
      Substance::UPtr(new Substance("L1", Color(0xFFFF0000))); // Color is red
  ecm->addArtificialGaussianConcentrationZ(L1.get(), 1.0, 400.0, 60.0);

  int number_of_nodes = 10;
  for (int i = 0; i < number_of_nodes; i++) {
    auto coord = Random::nextNoise(500);
    // ecm->createPhysicalNodeInstance(coord);
  }

  for (int i = 0; i < 8; i++) {
    Cell *c;
    double rand_1 = Random::nextDouble();
    double rand_2 = Random::nextDouble();
    if (i < 4) {
      c = CellFactory::getCellInstance(
          {-20 + 40 * rand_1, -20 + 40 * rand_2, 0.0});
      c->setNeuroMLType(Cell::NeuroMLType::kExcitatatory);
      c->setColorForAllPhysicalObjects(Param::kViolet);
    } else {
      c = CellFactory::getCellInstance(
          {-20 + 40 * rand_1, -20 + 40 * rand_2, 200.0});
      c->setNeuroMLType(Cell::NeuroMLType::kInhibitory);
      c->setColorForAllPhysicalObjects(
          Color(0xB38200AC)); // darker Param::kViolet
    }
    auto axon = c->getSomaElement()->extendNewNeurite();
    axon->setAxon(true);
    axon->getPhysicalCylinder()->setDiameter(0.5);
    axon->addLocalBiologyModule(
        LocalBiologyModule::UPtr{new NeuriteChemoAttraction("L1", 0.02)});

    if (i < 4) {
      axon->getPhysicalCylinder()->setColor(Param::kYellow);
    } else {
      axon->getPhysicalCylinder()->setColor(
          Color(0xB3B29415)); // darker Param::Yellow
    }

    auto dendrite = c->getSomaElement()->extendNewNeurite();
    dendrite->setAxon(false);
    dendrite->getPhysicalCylinder()->setDiameter(1.5);
    dendrite->addLocalBiologyModule(
        LocalBiologyModule::UPtr{new NeuriteChemoAttraction("L1", 0.02)});
  }


  auto scheduler = Scheduler::getInstance();
  auto max_time = 6;
  auto begin = std::chrono::steady_clock::now();
  while (ecm->getECMtime() < max_time) {
    auto middle = std::chrono::steady_clock::now();

    scheduler->simulateOneStep();

    auto end = std::chrono::steady_clock::now();
    double step = std::chrono::duration_cast<std::chrono::microseconds>(
                      end - middle).count() / 1e3;
    double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                    end - begin).count() / 1e6;
    printf("[Status] %2.1f %%, [elapsed] %2.1f s, [step] %3.1f ms\n",
           (ecm->getECMtime() * 100.0 / max_time), elapsed, step);
  }

  int objects = ecm->getPhysicalSphereListSize() + ecm->getPhysicalCylinderListSize();
  printf("[Info] Total objects in simulation: %d\n", objects);

  ConnectionMaker::extendExcressencesAndSynapseOnEveryNeuriteElement();

  auto beginUpd = std::chrono::steady_clock::now();
  visualization::GUI::getInstance().Update();
  auto endUpd = std::chrono::steady_clock::now();
  double vizTime = std::chrono::duration_cast<std::chrono::microseconds>(
      endUpd - beginUpd).count() / 1e3;

  printf("[Info] Total visualization time for one frame: %2.1f ms\n", vizTime);

  app.Run();
}