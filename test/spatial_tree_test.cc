#include "spatial/spatial_tree.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdlib.h>
#include "spatial/kd_tree.h"
#include "spatial/octree.h"
#include "test_util.h"

namespace bdm {

int manual_search_size(point *pos, int size, double distance) {
  int result = 0;
  double sd = distance * distance;
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      if (pos[i].distance(pos[j]) <= distance) result++;

  return result;
}

void size_test(spatial_tree_node<int> *tree, int amount) {
  point *possitions = new point[amount];
  double gap = 1.0 / (amount + 1);

  for (int i = 0; i < amount; i++) {
    possitions[i] = point(gap * i, rand() / RAND_MAX, rand() / RAND_MAX);
  }

  for (int i = 0; i < amount; i++) {
    tree->put(possitions[i], i);
  }

  ASSERT_EQ(manual_search_size(possitions, amount, 10),
            tree->get_neighbors(10)->size());
  ASSERT_EQ(manual_search_size(possitions, amount, 0.1),
            tree->get_neighbors(0.1)->size());
  ASSERT_EQ(manual_search_size(possitions, amount, 0.01),
            tree->get_neighbors(0.01)->size());

  delete[] possitions;
}

void simple_test(spatial_tree_node<int> *tree) {
  std::vector<int> container;
  tree->put(point(0, 0, 0), 1);
  tree->put(point(10, 20, 0), 2);
  tree->put(point(20, 10, 0), 3);

  auto result1 = tree->get_neighbors(15);
  auto result2 = tree->get_neighbors(30);
  auto result3 = tree->get_neighbors(5);

  ASSERT_EQ(1, result1->size());
  ASSERT_EQ(3, result2->size());
  ASSERT_EQ(0, result3->size());
}

TEST(SpatialTreeTest, OctreeTest) {
  spatial_tree_node<int> *tree =
      new octree_node<int>(bound(0.0, 0.0, 0.0, 100.0, 100.0, 100.0), 100, 100);
  simple_test(tree);
}

TEST(SpatialTreeTest, KdTest) {
  spatial_tree_node<int> *tree = new kd_tree_node<int>(
      bound(0.0, 0.0, 0.0, 100.0, 100.0, 100.0), 100, 100);
  simple_test(tree);
}

TEST(SpatialTreeTest, OctreeSizeTest) {
  spatial_tree_node<int> *tree =
      new octree_node<int>(bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  size_test(tree, 1000);
}

TEST(SpatialTreeTest, KdTreeSizeTest) {
  spatial_tree_node<int> *tree =
      new kd_tree_node<int>(bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  size_test(tree, 1000);
}

}  // namespace bdm
