#include "cell.h"
#include "diffusion_grid.h"
#include "grid.h"
#include "gtest/gtest.h"
#include "model_initializer.h"
#include "substance_initializers.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"

#include "Math/DistFunc.h"

namespace bdm {

enum Substances { kSubstance };

TEST(DiffusionInitTest, GaussianBand) {
  Param::Reset();
  ResourceManager<>::Get()->Clear();

  Param::bound_space_ = true;
  Param::min_bound_ = 0;
  Param::max_bound_ = 250;
  Rm()->Clear();

  // Create one cell at a random position
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateCellsRandom(Param::min_bound_, Param::max_bound_, 1,
                                      construct);

  // Define the substances in our simulation
  ModelInitializer::DefineSubstance(kSubstance, "Substance", 0.5, 0.1, 1);

  // Initialize the substance according to a GaussianBand along the x-axis
  ModelInitializer::InitializeSubstance(kSubstance, "Substance",
                                        GaussianBand(125, 50, Axis::kXAxis));

  auto& grid_ = Grid<>::GetInstance();
  grid_.Initialize();

  int lbound = Param::min_bound_;
  int rbound = Param::max_bound_;
  auto& dgrid = ResourceManager<>::Get()->GetDiffusionGrids()[0];

  // Create data structures, whose size depend on the grid dimensions
  dgrid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound},
                    grid_.GetBoxLength());
  // Initialize data structures with user-defined values
  dgrid->RunInitializers();

  array<uint32_t, 3> a = {0, 0, 0};
  array<uint32_t, 3> b = {25, 0, 0};
  array<uint32_t, 3> c = {13, 0, 0};
  array<uint32_t, 3> d = {0, 13, 0};
  array<uint32_t, 3> e = {25, 0, 13};
  array<uint32_t, 3> f = {13, 13, 13};

  auto kEps = abs_error<double>::value;
  auto conc = dgrid->GetAllConcentrations();

  EXPECT_NEAR(ROOT::Math::normal_pdf(0, 50, 125), conc[dgrid->GetBoxIndex(a)],
              kEps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(250, 50, 125), conc[dgrid->GetBoxIndex(b)],
              kEps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(130, 50, 125), conc[dgrid->GetBoxIndex(c)],
              kEps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(0, 50, 125), conc[dgrid->GetBoxIndex(d)],
              kEps);
  // Should be symmetric, so the two ends should have the same value
  EXPECT_NEAR(ROOT::Math::normal_pdf(0, 50, 125), conc[dgrid->GetBoxIndex(e)],
              kEps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(130, 50, 125), conc[dgrid->GetBoxIndex(f)],
              kEps);

  Param::Reset();
}

}  // namespace bdm
