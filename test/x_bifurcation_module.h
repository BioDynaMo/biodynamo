#ifndef TEST_X_BIFURCATION_MODULE_H_
#define TEST_X_BIFURCATION_MODULE_H_

#include <vector>
#include <array>
#include <string>
#include <memory>

#include "param.h"
#include "java_util.h"
#include "matrix.h"
#include "sim_state_serialization_util.h"

#include "cells/cell.h"
#include "cells/cell_factory.h"
#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"
#include "physics/physical_cylinder.h"
#include "simulation/ecm.h"
#include "simulation/scheduler.h"

namespace cx3d {

using cells::Cell;
using cells::CellFactory;
using local_biology::CellElement;
using local_biology::NeuriteElement;
using local_biology::LocalBiologyModule;
using physics::PhysicalCylinder;
using simulation::ECM;
using simulation::Scheduler;

/**
 * This class is used in Figure 8 and 9:
 * it defines a local biology module for bifurcation of the growth cone
 */
class XBifurcationModule : public LocalBiologyModule {
 public:
  XBifurcationModule(const std::shared_ptr<JavaUtil2>& java)
      : slope_ { 0.03 },
        shift_ { -0.01 },
        free_interval_ { 5 + 1 * java->getRandomDouble1() },
        java_ { java } {
  }
  XBifurcationModule(double slope, double shift, const std::shared_ptr<JavaUtil2>& java)
      : slope_ { slope },
        shift_ { shift },
        free_interval_ { 5 + 1 * java->getRandomDouble1() },
        java_ { java } {
  }
  XBifurcationModule(const XBifurcationModule&) = delete;
  XBifurcationModule& operator=(const XBifurcationModule&) = delete;

  void run() override {
    auto cyl = std::static_pointer_cast<PhysicalCylinder>(cell_element_->getPhysical());
    if (cyl->getActualLength() < free_interval_) {
      return;
    }

    // only terminal cylinders branch
    if (cyl->getDaughterLeft() != nullptr) {
      return;
    }

    // if not too thin
    if (cyl->getDiameter() < minimal_branch_diameter_) {
      return;
    }

    double totalConcentration = 0.0;
    for (auto s : branching_factors_) {
      double concentr = cyl->getExtracellularConcentration(s);
      totalConcentration += concentr;
    }

    if (totalConcentration < min_concentration_) {
      return;
    }
    double y = slope_ * totalConcentration + shift_;
    if (y > max_proba_) {
      y = max_proba_;
    }
    if (java_->getRandomDouble1() < y) {
      auto daughters = static_cast<NeuriteElement*>(cell_element_)->bifurcate();
      auto cyl0 = daughters[0]->getPhysicalCylinder();
      auto cyl1 = daughters[1]->getPhysicalCylinder();
      cyl0->setDiameter(cyl->getDiameter() * diameter_of_daughter_);
      cyl1->setDiameter(cyl->getDiameter() * diameter_of_daughter_);
    }
  }

  CellElement* getCellElement() const override {
    return cell_element_;
  }

  void setCellElement(CellElement* cell_element) override {
    cell_element_ = cell_element;
  }

  UPtr getCopy() const override {
    auto bf = std::unique_ptr<XBifurcationModule> { new XBifurcationModule(slope_, shift_, java_) };
    for (auto factor : branching_factors_) {
      bf->branching_factors_.push_back(factor);
    }
    bf->copied_when_neurite_branches_ = copied_when_neurite_branches_;
    bf->max_proba_ = max_proba_;
    bf->min_concentration_ = min_concentration_;
    bf->diameter_of_daughter_ = diameter_of_daughter_;
    bf->minimal_branch_diameter_ = minimal_branch_diameter_;
    return bf;
  }

  bool isCopiedWhenNeuriteBranches() const override {
    return copied_when_neurite_branches_;
  }

  bool isCopiedWhenSomaDivides() const override {
    return false;
  }

  bool isCopiedWhenNeuriteElongates() const override {
    return false;
  }

  bool isCopiedWhenNeuriteExtendsFromSoma() const override {
    return false;
  }

  bool isDeletedAfterNeuriteHasBifurcated() const override {
    return false;
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{}");
    return sb;
  }

  /** 1: no decrease in diameter, 0.5: decrease of fifty percent of the daughter branches.*/
  void setDiameterOfDaughter(double diameter) {
    diameter_of_daughter_ = diameter;
  }

  /* Diameter under which it will never branch.*/
  void setMinimalBranchDiameter(double diameter) {
    minimal_branch_diameter_ = diameter;
  }

  /** Add a chemical that will make this receptor branch.*/
  void addBranchingFactor(const std::string& bf) {
    branching_factors_.push_back(bf);
  }

  void setShift(double s) {
    shift_ = s;
  }

  /**
   * Specifies if this receptor is copied.
   * @param copiedWhenNeuriteBranches
   */
  void setCopiedWhenNeuriteBranches(bool copied) {
    copied_when_neurite_branches_ = copied;
  }

 private:
  std::shared_ptr<JavaUtil2> java_;

  /** The CellElement this module lives in.*/
  CellElement* cell_element_ = nullptr;

  /** Whether copied or not in branching.*/
  bool copied_when_neurite_branches_ = true;

  /** The chemical that activate branching.*/
  std::vector<std::string> branching_factors_;

  /** Slope of the probability to branch*/
  double slope_ = 0;

  /** shift in the probability */
  double shift_ = 0;

  double min_concentration_ = 0;

  double max_proba_ = 1;

  double diameter_of_daughter_ = 0.7;

  /** Stop if diameter smaller than this..*/
  double minimal_branch_diameter_ = 0.0;

  /** minimum interval before branching */
  double free_interval_;
};

}  // namespace cx3d

#endif  // TEST_X_BIFURCATION_MODULE_H_
