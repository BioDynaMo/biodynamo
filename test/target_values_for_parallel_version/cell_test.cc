#include "cells/cell.h"
#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "local_biology/soma_element.h"
#include "physics/physical_sphere.h"

namespace bdm {

using cells::Cell;
using local_biology::SomaElement;
using physics::PhysicalObject;
using physics::PhysicalSphere;

struct TestCell : public Cell {
  virtual ~TestCell() {}

  double expected_volume_ratio_;
  double expected_phi_;
  double expected_theta_;

  Cell* divide() override { return Cell::divide(); }

  Cell* divide(double volume_ratio) override {
    return Cell::divide(volume_ratio);
  }

  Cell* divide(const std::array<double, 3>& axis) override {
    return Cell::divide(axis);
  }

  Cell* divide(double volume_ratio,
               const std::array<double, 3>& axis) override {
    return Cell::divide(volume_ratio, axis);
  }

  /// override this method to capture forwarded parameters from overloaded
  /// divides. Other divide methods must be overriden as well, otherwise they
  /// would not be visible due to name hiding
  Cell* divide(double volume_ratio, double phi, double theta) override {
    EXPECT_NEAR(expected_volume_ratio_, volume_ratio, 1e-5);
    EXPECT_NEAR(expected_phi_, phi, 1e-5);
    EXPECT_NEAR(expected_theta_, theta, 1e-5);
    return nullptr;
  }
};

TEST(CellTest, divide) {
  TestCell* cell = new TestCell();
  Random::setSeed(42);

  cell->expected_volume_ratio_ = 1.0455127360065737;
  cell->expected_phi_ = 1.9633629889829609;
  cell->expected_theta_ = 4.2928196812086608;

  cell->divide();
}

TEST(CellTest, divideVolumeRatio) {
  TestCell* cell = new TestCell();
  Random::setSeed(42);

  cell->expected_volume_ratio_ = 0.59;
  cell->expected_phi_ = 1.1956088797871529;
  cell->expected_theta_ = 4.5714174264720571;

  cell->divide(0.59);
}

TEST(CellTest, divideAxis) {
  TestCell* cell = new TestCell();
  auto soma = std::unique_ptr<SomaElement>(new SomaElement());

  auto sphere = std::unique_ptr<PhysicalObject>(new PhysicalSphere());
  sphere->setMassLocation({1, 2, 3});

  soma->setPhysical(std::move(sphere));
  cell->setSomaElement(std::move(soma));
  Random::setSeed(42);

  cell->expected_volume_ratio_ = 1.0455127360065737;
  cell->expected_phi_ = 1.0442265974045177;
  cell->expected_theta_ = 0.72664234068172562;

  cell->divide({9, 8, 7});
}

TEST(CellTest, divideVolumeRatioAxis) {
  TestCell* cell = new TestCell();
  auto soma = std::unique_ptr<SomaElement>(new SomaElement());

  auto sphere = std::unique_ptr<PhysicalObject>(new PhysicalSphere());
  sphere->setMassLocation({1, 2, 3});

  soma->setPhysical(std::move(sphere));
  cell->setSomaElement(std::move(soma));
  Random::setSeed(42);

  cell->expected_volume_ratio_ = 0.456;
  cell->expected_phi_ = 1.0442265974045177;
  cell->expected_theta_ = 0.72664234068172562;

  cell->divide(0.456, {9, 8, 7});
}

}  // namespace bdm
