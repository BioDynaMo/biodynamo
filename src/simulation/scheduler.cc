#include "simulation/scheduler.h"

#include <iostream>

namespace bdm {
namespace simulation {

ECM* Scheduler::ecm_ = ECM::getInstance();

Scheduler* Scheduler::getInstance() {
  static std::unique_ptr<Scheduler> instance { new Scheduler() };
  return instance.get();
}

Scheduler::~Scheduler() {
}

void Scheduler::simulateOneStep() {

  if (print_current_ecm_time_) {
    std::cout << "time = " << ecm_->getECMtime() << std::endl;
  }

  if (print_current_step_) {
    std::cout << "step = " << cycle_counter_ << std::endl;
  }

  if (run_physics_) {
    // PhysicalNode (diffusion & degradation of Substances)
    if (run_diffusion_) {
      auto physicalNodeListSize = ecm_->getPhysicalNodeListSize();
      for (int i = 0; i < physicalNodeListSize; i++) {
        auto pn = ecm_->getPhysicalNode(i);
        if (pn->isOnTheSchedulerListForPhysicalNodes()) {
          pn->runExtracellularDiffusion();
        }
      }
    }

    //fixme implement - not used atm
//    if (!ecm_.ecm_ChemicalReactionList.isEmpty()) {
//      for (int i = 0; i < ecm_.physicalNodeList.size(); i++) {
//        ini.bdm.physics.interfaces.PhysicalNode
//        pn = ecm_.physicalNodeList.get(i);
//        for (ecm_ChemicalReaction chemicalReaction : ecm_.ecm_ChemicalReactionList) {
//          chemicalReaction.run(pn);
//        }
//      }
//    }

// Physical objects : PhysicalCylinders
    auto physicalCylinderListSize = ecm_->getPhysicalCylinderListSize();
    for (int i = 0; i < physicalCylinderListSize; i++) {
      auto pc = ecm_->getPhysicalCylinder(i);
      if (pc->isOnTheSchedulerListForPhysicalObjects()) {
        pc->runPhysics();
      }
      pc->runIntracellularDiffusion();
    }

    // Physical objects : PhysicalSpheres
    auto physicalSphereListSize = ecm_->getPhysicalSphereListSize();
    for (int i = 0; i < physicalSphereListSize; i++) {
      auto ps = ecm_->getPhysicalSphere(i);
      if (ps->isOnTheSchedulerListForPhysicalObjects()) {
        ps->runPhysics();
      }
      ps->runIntracellularDiffusion();
    }
  }
  // cellList

  // Modified by Sabina: the new cells should not be run in the same time step as they are created!!!
  int size = ecm_->getCellListSize();
  for (auto i = 0; i < size; i++) {
    ecm_->getCell(i)->run();
  }

  // soma
  auto somaElementListSize = ecm_->getSomaElementListSize();
  for (auto i = 0; i < somaElementListSize; i++) {
    ecm_->getSomaElement(i)->run();
  }
  // neurites
  auto neuriteElementListSize = ecm_->getNeuriteElementListSize();
  for (auto i = 0; i < neuriteElementListSize; i++) {
    ecm_->getNeuriteElement(i)->run();
  }

  // ticking ECM's time
  cycle_counter_++;
  ecm_->increaseECMtime(Param::kSimulationTimeStep);
}

void Scheduler::simulate() {
  while (true) {
    simulateOneStep();
  }
}

void Scheduler::simulateThatManyTimeSteps(int steps) {
  for (int i = 0; i < steps; i++) {
    simulateOneStep();
  }
}

void Scheduler::setPrintCurrentECMTime(bool print_time) {
  print_current_ecm_time_ = print_time;
}

Scheduler::Scheduler() {
}

}  // namespace simulation
}  // namespace bdm

