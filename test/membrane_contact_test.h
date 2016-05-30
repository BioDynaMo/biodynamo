#ifndef TEST_MEMBRANE_CONTACT_TEST_H_
#define TEST_MEMBRANE_CONTACT_TEST_H_

#include "param.h"

#include "base_simulation_test.h"
#include "soma_random_walk_module_test.h"
#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/local_biology_module.h"
#include "physics/intracellular_substance.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::AbstractLocalBiologyModule;
using local_biology::LocalBiologyModule;
using physics::IntracellularSubstance;
using simulation::ECM;
using simulation::Scheduler;

class MembraneContact : public AbstractLocalBiologyModule {
 public:
  MembraneContact() {
  }
  MembraneContact(const MembraneContact&) = delete;
  MembraneContact& operator=(const MembraneContact&) = delete;

  UPtr getCopy() const override {
    return UPtr { new MembraneContact() };
  }

  void run() override {
    auto physical = getCellElement()->getPhysical();
    for (auto o : physical->getPhysicalObjectsInContact()) {
      if (o->getMembraneConcentration("A") > 1) {
        physical->setColor(Param::kYellow);
        getCellElement()->cleanAllLocalBiologyModules();
      }
    }
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{}");
    return sb;
  }
};

class MembraneContactTest : public BaseSimulationTest {
 public:
  MembraneContactTest() {
  }

  void simulate() override {
    auto ecm = ECM::getInstance();
    Random::setSeed(1L);
    initPhysicalNodeMovementListener();

    auto adherence = IntracellularSubstance::UPtr(new IntracellularSubstance("A", 0, 0));
    adherence->setVisibleFromOutside(true);
    adherence->setVolumeDependant(false);
    ecm->addNewIntracellularSubstanceTemplate(std::move(adherence));

    ecm->setArtificialWallsForSpheres(true);
    ecm->setBoundaries(-150, 150, -150, 150, -100, 100);

    for (int i = 0; i < 10; i++) {
      auto c = CellFactory::getCellInstance(Random::nextNoise(100));
      c->setColorForAllPhysicalObjects(Param::kRed);
      c->getSomaElement()->getPhysical()->modifyMembraneQuantity("A", 100000);
    }
    for (int i = 0; i < 10; i++) {
      auto c = CellFactory::getCellInstance(Random::nextNoise(50));
      c->getSomaElement()->addLocalBiologyModule(LocalBiologyModule::UPtr { new MembraneContact() });
      c->getSomaElement()->addLocalBiologyModule(LocalBiologyModule::UPtr { new SomaRandomWalkModule() });
      c->setColorForAllPhysicalObjects(Param::kViolet);
    }

    auto scheduler = Scheduler::getInstance();
    for (int i = 0; i < 1500; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "MembraneContactTest";
  }
};

}  // namespace cx3d

#endif  // TEST_MEMBRANE_CONTACT_TEST_H_
