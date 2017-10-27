#ifndef UNIT_CELL_TEST_H_
#define UNIT_CELL_TEST_H_

#include <vector>

#include "biology_module_util.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "unit/test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace cell_test_internal {

struct GrowthModule : public BaseBiologyModule {
  double growth_rate_ = 0.5;
  GrowthModule() : BaseBiologyModule(gCellDivision) {}

  template <typename T>
  void Run(T* t) {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  ClassDefNV(GrowthModule, 1);
};

struct MovementModule {
  std::array<double, 3> velocity_;

  MovementModule() : velocity_({{0, 0, 0}}) {}
  explicit MovementModule(const std::array<double, 3>& velocity)
      : velocity_(velocity) {}

  template <typename T>
  void Run(T* t) {
    const auto& position = t->GetPosition();
    t->SetPosition(Matrix::Add(position, velocity_));
  }

  bool IsCopied(BmEvent event) const { return false; }
  ClassDefNV(MovementModule, 1);
};

template <typename TBackend = Soa>
struct CTParam {
  template <typename TTBackend>
  using Self = CTParam<TTBackend>;
  using Backend = TBackend;
  using BiologyModules = Variant<GrowthModule, MovementModule>;
};

/// Class used to get access to protected members
BDM_SIM_CLASS_TEST(TestCell, Cell, CTParam) {
  BDM_CLASS_HEADER(TestCellExt, 1, placeholder_);

 public:
  TestCellExt() {}

  void TestTransformCoordinatesGlobalToPolar() {
    array<double, 3> coord = {1, 2, 3};
    Base::SetMassLocation({9, 8, 7});
    auto result = Base::TransformCoordinatesGlobalToPolar(coord);

    EXPECT_NEAR(10.770329614269007, result[0], abs_error<double>::value);
    EXPECT_NEAR(1.9513027039072615, result[1], abs_error<double>::value);
    EXPECT_NEAR(-2.4980915447965089, result[2], abs_error<double>::value);
  }

  void SetXAxis(const array<double, 3>& axis) {
    Base::x_axis_[Base::kIdx] = axis;
  }
  void SetYAxis(const array<double, 3>& axis) {
    Base::y_axis_[Base::kIdx] = axis;
  }
  void SetZAxis(const array<double, 3>& axis) {
    Base::z_axis_[Base::kIdx] = axis;
  }

  const array<double, 3>& GetXAxis() { return Base::x_axis_[Base::kIdx]; }
  const array<double, 3>& GetYAxis() { return Base::y_axis_[Base::kIdx]; }
  const array<double, 3>& GetZAxis() { return Base::z_axis_[Base::kIdx]; }

  const std::vector<typename CTParam<>::BiologyModules>& GetBiologyModules()
      const {
    return Base::biology_modules_[0];
  }

  bool check_input_parameters_ = false;
  double expected_volume_ratio_;
  double expected_phi_;
  double expected_theta_;

  void DivideImpl(typename Base::template Self<Scalar> * daughter,
                  double volume_ratio, double phi, double theta) override {
    if (check_input_parameters_) {
      EXPECT_NEAR(expected_volume_ratio_, volume_ratio, 1e-8);
      EXPECT_NEAR(expected_phi_, phi, 1e-8);
      EXPECT_NEAR(expected_theta_, theta, 1e-8);
    } else {
      // forward call to implementation in CellExt
      Base::DivideImpl(daughter, volume_ratio, phi, theta);
    }
  }
  vec<bool> placeholder_;  // BDM_CLASS_HEADER needs at least one member
  FRIEND_TEST(CellTest, DivideVolumeRatioPhiTheta);
};

inline void RunIOTest() {
  // Temporary workaround for ROOT-8982; makes sure dictionary is working for
  // this type
  // important part is to add the namespace for the second template parameter:
  // bdm::SimulationObjectT
  bdm::CellExt<bdm::cell_test_internal::CTParam<bdm::Scalar>,
               bdm::SimulationObjectT>
      foo;

  remove(ROOTFILE);

  TestCell cell;
  cell.SetPosition({5, 6, 7});
  cell.SetMassLocation({5, 6, 7});
  cell.SetTractorForce({7, 4, 1});
  cell.SetDiameter(12);
  cell.UpdateVolume();
  cell.SetAdherence(1.1);
  cell.SetMass(5);
  cell.SetXAxis({1, 2, 3});
  cell.SetYAxis({4, 5, 6});
  cell.SetZAxis({7, 8, 9});
  cell.AddBiologyModule(GrowthModule());
  cell.AddBiologyModule(MovementModule({1, 2, 3}));
  cell.SetBoxIdx(123);

  // write to root file
  WritePersistentObject(ROOTFILE, "cell", cell, "new");

  // read back
  TestCell* restored_cell = nullptr;
  GetPersistentObject(ROOTFILE, "cell", restored_cell);

  // validate
  const double kEpsilon = abs_error<double>::value;
  EXPECT_NEAR(5, restored_cell->GetPosition()[0], kEpsilon);
  EXPECT_NEAR(6, restored_cell->GetPosition()[1], kEpsilon);
  EXPECT_NEAR(7, restored_cell->GetPosition()[2], kEpsilon);

  EXPECT_NEAR(5, restored_cell->GetMassLocation()[0], kEpsilon);
  EXPECT_NEAR(6, restored_cell->GetMassLocation()[1], kEpsilon);
  EXPECT_NEAR(7, restored_cell->GetMassLocation()[2], kEpsilon);

  EXPECT_NEAR(7, restored_cell->GetTractorForce()[0], kEpsilon);
  EXPECT_NEAR(4, restored_cell->GetTractorForce()[1], kEpsilon);
  EXPECT_NEAR(1, restored_cell->GetTractorForce()[2], kEpsilon);

  EXPECT_NEAR(12, restored_cell->GetDiameter(), kEpsilon);
  // differs slightly from the value in branch validation due to more precise
  // value of PI
  EXPECT_NEAR(cell.GetVolume(), restored_cell->GetVolume(), kEpsilon);
  EXPECT_NEAR(1.1, restored_cell->GetAdherence(), kEpsilon);
  EXPECT_NEAR(5, restored_cell->GetMass(), kEpsilon);

  EXPECT_NEAR(1, restored_cell->GetXAxis()[0], kEpsilon);
  EXPECT_NEAR(2, restored_cell->GetXAxis()[1], kEpsilon);
  EXPECT_NEAR(3, restored_cell->GetXAxis()[2], kEpsilon);

  EXPECT_NEAR(4, restored_cell->GetYAxis()[0], kEpsilon);
  EXPECT_NEAR(5, restored_cell->GetYAxis()[1], kEpsilon);
  EXPECT_NEAR(6, restored_cell->GetYAxis()[2], kEpsilon);

  EXPECT_NEAR(7, restored_cell->GetZAxis()[0], kEpsilon);
  EXPECT_NEAR(8, restored_cell->GetZAxis()[1], kEpsilon);
  EXPECT_NEAR(9, restored_cell->GetZAxis()[2], kEpsilon);

  EXPECT_EQ(2u, restored_cell->GetBiologyModules().size());
  EXPECT_TRUE(get_if<GrowthModule>(&restored_cell->GetBiologyModules()[0]) !=
              nullptr);
  EXPECT_NEAR(0.5, get_if<GrowthModule>(&restored_cell->GetBiologyModules()[0])
                       ->growth_rate_,
              kEpsilon);
  EXPECT_TRUE(get_if<MovementModule>(&restored_cell->GetBiologyModules()[1]) !=
              nullptr);

  EXPECT_EQ(123u, restored_cell->GetBoxIdx());

  // delete root file
  remove(ROOTFILE);
}

}  // namespace cell_test_internal
}  // namespace bdm

#endif  // UNIT_CELL_TEST_H_
