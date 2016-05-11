#ifndef TEST_X_MOVEMENT_MODULE_H_
#define TEST_X_MOVEMENT_MODULE_H_

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

using std::vector;
using std::array;
using std::string;
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
 * it defines a local module for moving the growth cone up a gradient
 */
class XMovementModule : public LocalBiologyModule {
 public:
  XMovementModule(const std::shared_ptr<JavaUtil2>& java)
      : movement_direction_ { { 0, 0, 0 } },
        java_ { java } {
  }
  XMovementModule(const XMovementModule&) = delete;
  XMovementModule& operator=(const XMovementModule&) = delete;

  void run() override {
    auto cyl = std::static_pointer_cast<PhysicalCylinder>(cell_element_->getPhysical());
    double length_before = cyl->getLength();
    // not to thin?
    if (cyl->getDiameter() < minimal_branch_diameter_) {
      if (cyl->lengthToProximalBranchingPoint() > 7 + 10 * java_->getRandomDouble1()) {  // so we don't end with short segments
        return;
      }
    }

    // find the gradients
    array<double, 3> total_gradient { 0.0, 0.0, 0.0 };
    for (auto s : attractants_) {
      auto normalized_grad = Matrix::normalize(cyl->getExtracellularGradient(s));
      total_gradient = Matrix::add(total_gradient, normalized_grad);
    }
    for (auto s : repellents_) {
      auto normalized_anti_grad = Matrix::scalarMult(-1.0, Matrix::normalize(cyl->getExtracellularGradient(s)));
      total_gradient = Matrix::add(total_gradient, normalized_anti_grad);
    }
    // if no gradient : go straight
    if (attractants_.empty() && repellents_.empty()) {
      total_gradient = movement_direction_;
    }

    auto noise = java_->matrixRandomNoise3(randomness_);
    auto new_step_direction = Matrix::add(
        Matrix::add(movement_direction_, Matrix::scalarMult(direction_weight_, total_gradient)), noise);

    cyl->movePointMass(speed_, new_step_direction);

    movement_direction_ = Matrix::normalize(
        Matrix::add(Matrix::scalarMult(5, movement_direction_), new_step_direction));

    // decrease diameter setDiameter
    double length_after = cyl->getLength();
    double delta_l = length_after - length_before;
    if (delta_l < 0)
      delta_l = 0;
    cyl->setDiameter(cyl->getDiameter() * (1 - delta_l * linear_diameter_decrease_));
  }

  std::shared_ptr<CellElement> getCellElement() const override {
    return cell_element_;
  }

  void setCellElement(const std::shared_ptr<CellElement>& cell_element) override {
    cell_element_ = cell_element;
    movement_direction_ = cell_element_->getPhysical()->getXAxis();
  }

  UPtr getCopy() const override {
    auto r = std::unique_ptr<XMovementModule> { new XMovementModule(java_) };
    for (auto el : attractants_) {
      r->attractants_.push_back(el);
    }
    for (auto el : repellents_) {
      r->repellents_.push_back(el);
    }
    r->randomness_ = randomness_;
    r->movement_direction_ = movement_direction_;
    r->direction_weight_ = direction_weight_;
    r->copied_when_neurite_branches_ = copied_when_neurite_branches_;
    r->minimal_branch_diameter_ = minimal_branch_diameter_;
    r->max_concentration_ = max_concentration_;
    r->linear_diameter_decrease_ = linear_diameter_decrease_;
    return r;
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

  /**
   * Specifies if this receptor is copied.
   * @param copiedWhenNeuriteBranches
   */
  void setCopiedWhenNeuriteBranches(bool copied) {
    copied_when_neurite_branches_ = copied;
  }

  /** Add a chemical this receptor will follows.*/
  void addAttractant(const string& attractant) {
    attractants_.push_back(attractant);
  }

  /** Add a chemical this receptor will avoid.*/
  void addRepellent(const string& repellent) {
    repellents_.push_back(repellent);
  }

  // getters and setters
  double getMaxConcentration() const {
    return max_concentration_;
  }

  void setMaxConcentration(double max) {
    max_concentration_ = max;
  }

  double getMinimalBranchDiameter() const {
    return minimal_branch_diameter_;
  }

  void setMinimalBranchDiameter(double min) {
    minimal_branch_diameter_ = min;
  }

  double getLinearDiameterDecrease() const {
    return linear_diameter_decrease_;
  }

  void setLinearDiameterDecrease(double decrease) {
    linear_diameter_decrease_ = decrease;
  }

  double getRandomness() const {
    return randomness_;
  }

  void setRandomness(double r) {
    randomness_ = r;
  }

  double getSpeed() const {
    return speed_;
  }

  void setSpeed(double s) {
    speed_ = s;
  }

 private:
  std::shared_ptr<JavaUtil2> java_;

  /** The CellElement this module lives in.*/
  std::shared_ptr<CellElement> cell_element_;

  /** Whether copied or not in branching.*/
  bool copied_when_neurite_branches_ = true;

  /** The chemical this Receptor is attracted by.*/
  vector<string> attractants_;

  /** The chemical this Receptor is repelled by.*/
  vector<string> repellents_;

  /** The chemical this Receptor dies if it goes opposite to.*/
  double max_concentration_ = 0;

  /** Stop if diameter smaller than this..*/
  double minimal_branch_diameter_ = 0.7;

  double linear_diameter_decrease_ = 0.001;

  array<double, 3> movement_direction_;

  double randomness_ = 0.3;

  double direction_weight_ = 0.1;

  double speed_ = 100;
};

}  // namespace cx3d

#endif  // TEST_X_MOVEMENT_MODULE_H_
