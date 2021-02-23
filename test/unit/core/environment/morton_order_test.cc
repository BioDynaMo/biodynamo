
#include "core/environment/morton_order.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(MortonOrder, Cube1) {
  MortonOrder mo;
  mo.Update({1, 1, 1});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {};
  EXPECT_EQ(expected, mo.offset_index_);
}

TEST(MortonOrder, Cube2) {
  MortonOrder mo;
  mo.Update({2, 2, 2});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {};
  EXPECT_EQ(expected, mo.offset_index_);
}

TEST(MortonOrder, Cube1024) {
  MortonOrder mo;
  mo.Update({1024, 1024, 1024});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {};
  EXPECT_EQ(expected, mo.offset_index_);
}

/// Not power of 2
TEST(MortonOrder, Cube3) {
  MortonOrder mo;
  mo.Update({3, 3, 3});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {
      {9, 0},   {11, 1},  {13, 2},  {15, 3},  {18, 4},  {22, 6}, {25, 8},
      {29, 11}, {36, 14}, {41, 18}, {43, 19}, {50, 24}, {57, 30}};
  EXPECT_EQ(expected, mo.offset_index_);
}

TEST(MortonOrder, 135) {
  MortonOrder mo;
  mo.Update({1, 3, 5});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {
      {1, 0},   {3, 1},   {5, 2},     {7, 3},     {17, 12},
      {21, 15}, {33, 26}, {35, 27},   {37, 28},   {39, 29},
      {49, 38}, {53, 41}, {257, 244}, {259, 245}, {273, 258}};
  EXPECT_EQ(expected, mo.offset_index_);
}

TEST(MortonOrder, 537) {
  MortonOrder mo;
  mo.Update({5, 3, 7});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {
      {18, 0},    {22, 2},    {26, 4},    {30, 6},    {50, 8},    {54, 10},
      {58, 12},   {62, 14},   {65, 16},   {67, 17},   {69, 18},   {71, 19},
      {81, 28},   {85, 31},   {97, 42},   {99, 43},   {101, 44},  {103, 45},
      {113, 54},  {117, 57},  {274, 196}, {278, 198}, {282, 200}, {286, 202},
      {292, 204}, {300, 208}, {306, 212}, {314, 218}, {321, 224}, {323, 225},
      {325, 226}, {327, 227}, {337, 236}, {341, 239}, {353, 250}, {355, 251},
      {369, 264}};

  EXPECT_EQ(expected, mo.offset_index_);
}

}  // namespace bdm
