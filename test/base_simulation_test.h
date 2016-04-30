#ifndef BASE_SIMULATION_TEST_H_
#define BASE_SIMULATION_TEST_H_

#include <string>
#include <memory>
#include <chrono>
#include <iostream>

#include "gtest/gtest.h"

#include "java_util.h"
#include "cells/cell.h"
#include "local_biology/cell_element.h"
#include "physics/physical_node.h"
#include "physics/physical_object.h"
#include "physics/default_force.h"
#include "simulation/ecm.h"
#include "spatial_organization/space_node.h"
#include "synapse/excrescence.h"

namespace cx3d {

using std::string;
using cells::Cell;
using local_biology::CellElement;
using simulation::ECM;
using physics::PhysicalNode;
using physics::DefaultForce;
using physics::PhysicalObject;
using spatial_organization::SpaceNode;
using synapse::Excrescence;

class BaseSimulationTest {
 public:
  BaseSimulationTest() {
  }

  virtual ~BaseSimulationTest() {
  }

  void run() {
    configure();
    long start = timestamp();
    simulate(ECM::getInstance(), std::shared_ptr<JavaUtil2>(new JavaUtil2()));
    long end = timestamp();
    std::cout << getTestName() << " simulation runtime: " << (end - start) << "ms" << std::endl;
    assertSimulationState();
  }

 protected:
  virtual void simulate(const std::shared_ptr<ECM>& ecm, const std::shared_ptr<JavaUtil2>& java) = 0;

  virtual string getTestName() const = 0;

  void configure() {
    std::shared_ptr<JavaUtil<PhysicalNode>> java1 { new JavaUtil<PhysicalNode>() };
    std::shared_ptr<JavaUtil2> java_ { new JavaUtil2() };
    auto ecm = ECM::getInstance();

    ECM::setJavaUtil(java_);
    CellElement::setECM(ecm);
    PhysicalNode::setECM(ecm);
    Excrescence::setECM(ecm);
    Cell::setECM(ecm);
    SpaceNode<PhysicalNode>::setJavaUtil(java1);
    DefaultForce::setJavaUtil(java_);
    PhysicalObject::setInterObjectForce(DefaultForce::create());
  }

 private:
  void assertSimulationState() {
    FAIL();
  }

  long timestamp() {
    namespace sc = std::chrono;
    auto time = sc::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = sc::duration_cast<sc::milliseconds>(since_epoch);
    return millis.count();
  }

};

}  // namespace cx3d

#endif  // BASE_SIMULATION_TEST_H_
