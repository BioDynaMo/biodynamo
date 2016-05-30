#ifndef TEST_NEURITE_CHEMO_ATTRACTION_TEST_H_
#define TEST_NEURITE_CHEMO_ATTRACTION_TEST_H_

#include <array>
#include <memory>
#include <string>

#include "base_simulation_test.h"
#include "param.h"
#include "matrix.h"


#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "physics/substance.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::AbstractLocalBiologyModule;
using local_biology::LocalBiologyModule;
using physics::PhysicalNode;
using physics::Substance;
using simulation::ECM;
using simulation::Scheduler;

class NeuriteChemoAttraction : public AbstractLocalBiologyModule {
 public:
  NeuriteChemoAttraction(const std::string& substance_id)
      : direction_ { { 0.0, 0.0, 0.0 } },
        substance_id_ { substance_id } {
  }
  NeuriteChemoAttraction(const std::string& substance_id, double branching_factor)
      : direction_ { { 0.0, 0.0, 0.0 } },
        substance_id_ { substance_id },
        branching_factor_ { branching_factor } {
  }
  NeuriteChemoAttraction(const NeuriteChemoAttraction&) = delete;
  NeuriteChemoAttraction& operator=(const NeuriteChemoAttraction&) = delete;

  UPtr getCopy() const override {
    return UPtr { new NeuriteChemoAttraction(substance_id_) };
  }

  bool isCopiedWhenNeuriteBranches() const override {
    return true;
  }

  bool isDeletedAfterNeuriteHasBifurcated() const override {
    return true;
  }

  void setCellElement(CellElement* cell_element) override {
    AbstractLocalBiologyModule::setCellElement(cell_element);
    if (cell_element->isANeuriteElement()) {
      direction_ = cell_element->getPhysical()->getAxis();
    }
  }

  void run() override {
    auto physical = getCellElement()->getPhysical();
    double concentration = physical->getExtracellularConcentration(substance_id_);
    auto grad = physical->getExtracellularGradient(substance_id_);

    // 1) movement
    if (physical->getExtracellularConcentration(substance_id_) > 0.3) {
      grad = {0.0, 0.0, 0.0};
    }

    auto noise = Random::nextNoise(kRandomnessWeight);
    auto scaled_direction = Matrix::scalarMult(kOldDirectionWeight, direction_);
    auto scaled_normalized_grad = Matrix::scalarMult(kGradientWeight, Matrix::normalize(grad));
    auto new_dir = Matrix::add(Matrix::add(scaled_direction, scaled_normalized_grad), noise);
    double speed = 100;
    physical->movePointMass(speed, new_dir);

    direction_ = Matrix::normalize(Matrix::add(Matrix::scalarMult(5, direction_), new_dir));

    // 2) branching based on concentration:
    if (Random::nextDouble() < concentration * branching_factor_) {
      static_cast<NeuriteElement*>(getCellElement())->bifurcate();
    }
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{}");
    return sb;
  }

 private:
  static constexpr double kOldDirectionWeight = 1.0;
  static constexpr double kGradientWeight = 0.2;
  static constexpr double kRandomnessWeight = 0.6;

  std::array<double, 3> direction_;
  double branching_factor_ = 0.005;
  std::string substance_id_;
};

class NeuriteChemoAttractionTest : public BaseSimulationTest {
 public:
  NeuriteChemoAttractionTest() {
  }

  void simulate() override {
    auto ecm = ECM::getInstance();
    Random::setSeed(1L);
    initPhysicalNodeMovementListener();

    auto attractant = Substance::UPtr(new Substance("A", Color(0xFFFF0000)));
    ecm->addArtificialGaussianConcentrationZ(attractant.get(), 1.0, 400.0, 160.0);

    int number_of_additional_nodes = 10;
    for (int i = 0; i < number_of_additional_nodes; i++) {
      auto coord = Random::nextNoise(500);
      physical_nodes_.push_back(ecm->createPhysicalNodeInstance(coord));
    }

    auto c = CellFactory::getCellInstance( { 0.0, 0.0, 0.0 });
    c->setColorForAllPhysicalObjects(Param::kViolet);
    auto neurite = c->getSomaElement()->extendNewNeurite();
    neurite->getPhysicalCylinder()->setDiameter(2.0);
    neurite->addLocalBiologyModule( LocalBiologyModule::UPtr { new NeuriteChemoAttraction("A") });

    auto scheduler = Scheduler::getInstance();
    for (int i = 0; i < 1000; i++) {
      scheduler->simulateOneStep();
    }
  }

  std::string getTestName() const override {
    return "NeuriteChemoAttractionTest";
  }
};

}  // namespace cx3d

#endif  // TEST_NEURITE_CHEMO_ATTRACTION_TEST_H_
