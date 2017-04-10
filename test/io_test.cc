#include "gtest/gtest.h"

#include <TFile.h>

#include "cell.h"
#include "displacement_op.h"
#include "neighbor_op.h"
#include "test_util.h"

namespace bdm {
TEST(IOTest, PersistAndLoadVc) {
  daosoa<Cell> cells;
  daosoa<Cell> *cells_r;

  Cell<VcBackend> cell(std::array<VcBackend::real_v, 3>{0, 0, 0});
  cells.push_back(cell);

  for (size_t d = 0; d < 3; d++) {
    std::array<VcBackend::real_v, 3> current_pos = cells[0].GetPosition();
    std::array<VcBackend::real_v, 3> new_pos{(current_pos[0] + 1.0),
                                             current_pos[1], current_pos[2]};
    cells[0].SetPosition(new_pos);

    cells.PersistData();

    TFile *write = new TFile("bdm_state.root", "RECREATE");
    write->WriteObject(&cells, "daosoa_object");
    write->Close();

    TFile *read = TFile::Open("bdm_state.root");
    read->GetObject("daosoa_object", cells_r);
    read->Close();

    cells_r->LoadData();
  }

  auto &final_position_r = (*cells_r)[0].GetPosition();
  EXPECT_NEAR(3, final_position_r[0][0], abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position_r[1][0], abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position_r[2][0], abs_error<real_t>::value);
}

TEST(IOTest, PersistedDisplacementOp) {
  using real_v = VcBackend::real_v;
  using real_t = real_v::value_type;
  if (real_v::Size < 2) {
    FAIL() << "Backend must at least support two elements for this test";
  }

  // set up cells
  real_v diameter((const real_t[]){9, 11});
  real_v adherence((const real_t[]){0.3, 0.4});
  real_v mass((const real_t[]){1.4, 1.1});

  std::array<real_v, 3> position = {real_v((const real_t[]){0, 0}),
                                    real_v((const real_t[]){0, 5}),
                                    real_v((const real_t[]){0, 0})};

  InlineVector<int, 8> neighbor_1;
  neighbor_1.push_back(1);
  InlineVector<int, 8> neighbor_2;
  neighbor_2.push_back(0);
  std::array<InlineVector<int, 8>, VcBackend::kVecLen> neighbors = {neighbor_1,
                                                                    neighbor_2};

  Cell<VcBackend> cell(diameter);
  cell.SetDiameter(diameter);
  cell.SetPosition(position);
  cell.SetMassLocation(position);
  cell.SetAdherence(adherence);
  cell.SetMass(mass);
  cell.SetNeighbors(neighbors);
  daosoa<Cell, VcBackend> cells;
  cells.push_back(cell);

  // execute operation
  DisplacementOp op;
  op.Compute(&cells);

  // Copy data to persistent members
  cells.PersistData();

  // Persist cells object to ROOT file
  TFile *write = new TFile("bdm_state.root", "RECREATE");
  write->WriteObject(&cells, "daosoa_object");
  write->Close();

  // Load back cells object from ROOT file
  daosoa<Cell, VcBackend> *cells_r;
  TFile *read = TFile::Open("bdm_state.root");
  read->GetObject("daosoa_object", cells_r);
  read->Close();

  // Load data from persistent members
  cells_r->LoadData();

  // check results persistent data member
  auto &final_position_r = (*cells_r)[0].GetPosition();
  // cell 1
  EXPECT_NEAR(0, final_position_r[0][0], abs_error<real_t>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position_r[1][0],
              abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position_r[2][0], abs_error<real_t>::value);
  // cell 2
  EXPECT_NEAR(0, final_position_r[0][1], abs_error<real_t>::value);
  EXPECT_NEAR(5.0992371702325645, final_position_r[1][1],
              abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position_r[2][1], abs_error<real_t>::value);

  // check if tractor_force has been reset to zero
  auto &final_tf_r = (*cells_r)[0].GetTractorForce();
  EXPECT_NEAR(0, final_tf_r[0].sum(), abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf_r[1].sum(), abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf_r[2].sum(), abs_error<real_t>::value);

  // remaining fields should remain unchanged
  EXPECT_TRUE((diameter == (*cells_r)[0].GetDiameter()).isFull());
  EXPECT_TRUE((adherence == (*cells_r)[0].GetAdherence()).isFull());
  EXPECT_TRUE((mass == (*cells_r)[0].GetMass()).isFull());
}

TEST(IOTest, PersistedNeighborOp) {
  daosoa<Cell, VcBackend> cells;
  // fixme ugly
  cells.push_back(
      Cell<ScalarBackend>(std::array<ScalarBackend::real_v, 3>{0, 0, 0}));
  cells.push_back(
      Cell<ScalarBackend>(std::array<ScalarBackend::real_v, 3>{30, 30, 30}));
  cells.push_back(
      Cell<ScalarBackend>(std::array<ScalarBackend::real_v, 3>{60, 60, 60}));

  // execute operation
  NeighborOp op;
  op.Compute(&cells);

  cells.PersistData();

  // Persist cells object to ROOT file
  TFile *write = new TFile("bdm_state.root", "RECREATE");
  write->WriteObject(&cells, "daosoa_object");
  write->Close();

  // Load back cells object from ROOT file
  daosoa<Cell, VcBackend> *cells_r;
  TFile *read = TFile::Open("bdm_state.root");
  read->GetObject("daosoa_object", cells_r);
  read->Close();

  cells_r->LoadData();

  // check results for persisted object
  // cell 1
  auto &neighbors_1_r = (*cells_r)[0].GetNeighbors();
  InlineVector<int, 8> expected_1_r;
  expected_1_r.push_back(1);
  EXPECT_TRUE(expected_1_r == neighbors_1_r[0]);
  // cell 2
  InlineVector<int, 8> expected_2_r;
  expected_2_r.push_back(0);
  expected_2_r.push_back(2);
  EXPECT_EQ(expected_2_r, neighbors_1_r[1]);
  // cell 3
  InlineVector<int, 8> expected_3_r;
  expected_3_r.push_back(1);
  if (VcBackend::kVecLen > 2) {
    EXPECT_EQ(expected_3_r, neighbors_1_r[2]);
  } else {
    EXPECT_EQ(expected_3_r, (*cells_r)[1].GetNeighbors()[0]);
  }
}
}  // namespace bdm
