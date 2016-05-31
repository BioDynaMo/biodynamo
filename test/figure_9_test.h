#ifndef TEST_FIGURE_9_TEST_H_
#define TEST_FIGURE_9_TEST_H_

#include <array>
#include <memory>

#include "base_simulation_test.h"
#include "x_adhesive_force.h"
#include "x_bifurcation_module.h"
#include "x_movement_module.h"
#include "param.h"


#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "physics/physical_object.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"
#include "synapse/test_synapses.h"


namespace bdm {

using std::array;
using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::LocalBiologyModule;
using physics::PhysicalNode;
using simulation::ECM;
using simulation::Scheduler;
using physics::PhysicalObject;
using synapse::TestSynapses;

class Figure9Test : public BaseSimulationTest {
 public:
  Figure9Test() {
  }

  void simulate() override {
    auto ecm = ECM::getInstance();
    Param::kNeuriteMaxLength = 20;
    double pi = Param::kPi;
    // get a 2.5D ECM
    Random::setSeed(5L);
    initPhysicalNodeMovementListener();
    ecm->setArtificialWallsForCylinders(true);
    ecm->setArtificialWallsForSpheres(true);
    ecm->setBoundaries(-10000, 10000, -10000, 10000, -5, 5);

    for (int i = 0; i < 12; i++) {
      array<double, 3> loc;
      loc[0] = -600 + 2 * 600 * Random::nextDouble();
      loc[1] = -600 + 2 * 600 * Random::nextDouble();
      loc[2] = -100 + 2 * 100 * Random::nextDouble();
      physical_nodes_.push_back(ecm->createPhysicalNodeInstance(loc));
    }

    // set the inter object force
    auto nogo = std::unique_ptr<XAdhesiveForce>(new XAdhesiveForce());
    nogo->setAttractionRange(3);
    nogo->setAttractionStrength(5);

    PhysicalObject::setInterObjectForce(std::move(nogo));

    // generate cells
    int nb_of_cells = 20;
    int min_nb_of_neurites = 4;
    int max_nb_of_neurites = 8;

    for (int i = 0; i < nb_of_cells; i++) {

      Color c = Param::kGraySolid;

      double rand_1 = Random::nextDouble();
      double rand_2 = Random::nextDouble();
      double rand_3 = Random::nextDouble();
      array<double, 3> cell_location { -200 + rand_1 * 400, -200 + rand_2 * 400, -5 + rand_3 * 10 };

      if (i == 0) {
        c = Param::kRedSolid;
        cell_location = {0,0,0};
      }

      auto cell = CellFactory::getCellInstance(cell_location);
      auto soma = cell->getSomaElement();
      auto sphere = soma->getPhysicalSphere();
      if (i == 0) {
        cell->setNeuroMLType(Cell::NeuroMLType::kInhibitory);
      } else {
        cell->setNeuroMLType(Cell::NeuroMLType::kExcitatatory);
      }
      sphere->setColor(c);
      sphere->setAdherence(100);

      int nb_of_neurites = min_nb_of_neurites
          + ((int) ((max_nb_of_neurites - min_nb_of_neurites) * Random::nextDouble()));

      for (int j = 0; j < nb_of_neurites; j++) {
        double angle_of_axon = pi * 2 * Random::nextDouble();
        double growth_speed = 75;
        double branch_probability = 0.003;
        double linearDiameterDecrease = 0.001;
        NeuriteElement* ne = nullptr;
        if (j == 0) {
          ne = cell->getSomaElement()->extendNewNeurite(3.0, Param::kPi * 0.5, angle_of_axon);
          ne->setAxon(true);
          growth_speed = 150;
          branch_probability = 0.009;
          linearDiameterDecrease = 0;
          ne->getPhysicalCylinder()->setDiameter(1.5);
        } else if (j == 1) {
          ne = cell->getSomaElement()->extendNewNeurite(3.0, Param::kPi * 0.5,
                                                        angle_of_axon + Param::kPi - 0.5 + Random::nextDouble());
          ne->setAxon(false);
        } else {
          ne = cell->getSomaElement()->extendNewNeurite(3.0, Param::kPi * 0.5,
                                                        Param::kPi * 2 * Random::nextDouble());
          ne->setAxon(false);
        }

        auto br = std::unique_ptr<XBifurcationModule> { new XBifurcationModule() };
        br->setShift(branch_probability);
        ne->addLocalBiologyModule(std::move(br));
        auto mr = std::unique_ptr<XMovementModule> { new XMovementModule() };
        mr->setRandomness(0.7);
        mr->setSpeed(growth_speed);
        mr->setLinearDiameterDecrease(linearDiameterDecrease);
        ne->addLocalBiologyModule(std::move(mr));
      }
    }
    auto scheduler = Scheduler::getInstance();
    for (int i = 0; i < 350; i++) {  // 350
      scheduler->simulateOneStep();
    }

    TestSynapses::extendExcressencesAndSynapseOnEveryNeuriteElement(0.4);
  }

  std::string getTestName() const override {
    return "Figure9Test";
  }
};

}  // namespace bdm

#endif  // TEST_FIGURE_9_TEST_H_
