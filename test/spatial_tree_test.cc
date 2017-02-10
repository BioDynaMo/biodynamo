#include "spatial/spatial_tree.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdlib.h>
#include "spatial/kd_tree.h"
#include "spatial/octree.h"
#include "test_util.h"

namespace bdm {

int ManualSearchSize(point *pos, int size, double distance) {
  int result = 0;
  double sd = distance * distance;
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      if (pos[i].EuclidianDistance(pos[j]) <= distance) result++;

  return result;
}

std::vector<std::pair<int, int>> ManualSearch(point *pos, int size,
                                              double distance) {
  std::vector<std::pair<int, int>> result;
  double sd = distance * distance;
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      if (pos[i].EuclidianDistance(pos[j]) <= distance) {
        result.push_back(std::make_pair(i, j));
      }

  return result;
}

void SizeTest(spatial_tree_node<int> *tree, int amount) {
  point *possitions = new point[amount];
  double gap = 1.0 / (amount + 1);

  for (int i = 0; i < amount; i++) {
    possitions[i] = point(gap * i, rand() / RAND_MAX, rand() / RAND_MAX);
  }

  for (int i = 0; i < amount; i++) {
      tree->Put(possitions[i], i);
  }

  ASSERT_EQ(ManualSearchSize(possitions, amount, 10),
            tree->GetNeighbors(10).size());
  ASSERT_EQ(ManualSearchSize(possitions, amount, 0.1),
            tree->GetNeighbors(0.1).size());
  ASSERT_EQ(ManualSearchSize(possitions, amount, 0.01),
            tree->GetNeighbors(0.01).size());

  delete[] possitions;
}

void SimpleTest(spatial_tree_node<int> *tree) {
  std::vector<int> container;
    tree->Put(point(0, 0, 0), 1);
    tree->Put(point(10, 20, 0), 2);
    tree->Put(point(20, 10, 0), 3);

  auto result1 = tree->GetNeighbors(15);
  auto result2 = tree->GetNeighbors(30);
  auto result3 = tree->GetNeighbors(5);

  ASSERT_EQ(1, result1.size());
  ASSERT_EQ(3, result2.size());
  ASSERT_EQ(0, result3.size());
}

bool IsEqual(std::vector<std::pair<int, int>> a,
             std::vector<std::pair<int, int>> b) {
  if (a.size() != b.size()) {
    return false;
  }
  std::sort(a.begin(), a.end(), std::greater<std::pair<int, int>>());
  std::sort(b.begin(), b.end(), std::greater<std::pair<int, int>>());

  for (int i = 0; i < a.size(); i++) {
    if (a[i].first != b[i].first || a[i].second != b[i].second) {
      return false;
    }
  }
  return true;
}

bool SearchTest(spatial_tree_node<int> *tree, int amount) {
  point *possitions = new point[amount];
  double gap = 1.0 / (amount + 1);
  for (int i = 0; i < amount; i++) {
    possitions[i] = point(gap * i, rand() / RAND_MAX, rand() / RAND_MAX);
  }

  for (int i = 0; i < amount; i++) {
      tree->Put(possitions[i], i);
  }

  double search_radious = 0.1;
  for (int i = 0; i < 2; i++, search_radious /= 10) {
    std::vector<std::pair<int, int>> manual_result =
            ManualSearch(possitions, amount, search_radious);
    std::vector<std::pair<int, int>> tree_search =
            tree->GetNeighbors(search_radious);
  }

  delete[] possitions;
}

TEST(SpatialTreeTest, OctreeTest) {
  spatial_tree_node<int> *tree =
      new octree_node<int>(bound(0.0, 0.0, 0.0, 100.0, 100.0, 100.0), 100, 100);
  SimpleTest(tree);
  delete tree;
}

TEST(SpatialTreeTest, KdTest) {
  spatial_tree_node<int> *tree = new kd_tree_node<int>(
      bound(0.0, 0.0, 0.0, 100.0, 100.0, 100.0), 100, 100);
  SimpleTest(tree);
  delete tree;
}

TEST(SpatialTreeTest, OctreeSizeTest) {
  spatial_tree_node<int> *tree =
      new octree_node<int>(bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  SizeTest(tree, 1000);
  delete tree;
}

TEST(SpatialTreeTest, KdTreeSizeTest) {
  spatial_tree_node<int> *tree =
      new kd_tree_node<int>(bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  SizeTest(tree, 1000);
  delete tree;
}

TEST(SpatialTreeTest, OctreeSearchTest) {
  spatial_tree_node<int> *tree =
      new octree_node<int>(bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  SearchTest(tree, 1000);
  delete tree;
}

TEST(SpatialTreeTest, KdTreeSearchTest) {
  spatial_tree_node<int> *tree =
      new kd_tree_node<int>(bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  SearchTest(tree, 1000);
  delete tree;
}

}  // namespace bdm
