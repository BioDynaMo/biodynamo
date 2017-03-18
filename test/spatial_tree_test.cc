#include <gtest/gtest.h>
#include <stdlib.h>
#include <random>

#include "spatial/kd_tree_node.h"
#include "spatial/octree_node.h"
#include "spatial/spatial_tree_node.h"

namespace bdm {

int ManualSearchSize(Point *pos, int size, double distance) {
  int result = 0;
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      if (pos[i].EuclidianDistance(pos[j]) <= distance) result++;

  return result;
}

std::vector<std::pair<int, int>> ManualSearch(Point *pos, int size,
                                              double distance) {
  std::vector<std::pair<int, int>> result;
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      if (pos[i].EuclidianDistance(pos[j]) <= distance) {
        result.push_back(std::make_pair(i, j));
      }

  return result;
}

void SizeTest(SpatialTreeNode<int> *tree, int amount) {
  Point *possitions = new Point[amount];
  double gap = 1.0 / (amount + 1);

  std::minstd_rand simple_rand;
  simple_rand.seed(42);

  for (int i = 0; i < amount; i++) {
    possitions[i] = Point(gap * i, simple_rand() / simple_rand.max(),
                          simple_rand() / simple_rand.max());
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

void SimpleTest(SpatialTreeNode<int> *tree) {
  std::vector<int> container;
  tree->Put(Point(0, 0, 0), 1);
  tree->Put(Point(10, 20, 0), 2);
  tree->Put(Point(20, 10, 0), 3);

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
  std::vector<std::pair<int, int>> a_copy, b_copy;

  for (int i = 0; i < a.size(); i++) {
    if (a[i].first > a[i].second) {
      a_copy.push_back(a[i]);
    } else {
      a_copy.push_back(make_pair(a[i].second, a[i].first));
    }
    if (b[i].first > b[i].second) {
      b_copy.push_back(b[i]);
    } else {
      b_copy.push_back(make_pair(b[i].second, b[i].first));
    }
  }

  auto comparator = std::greater<std::pair<int, int>>();
  std::sort(a_copy.begin(), a_copy.end(), comparator);
  std::sort(b_copy.begin(), b_copy.end(), comparator);

  for (unsigned int i = 0; i < a.size(); i++) {
    if (a_copy[i].first != b_copy[i].first ||
        a_copy[i].second != b_copy[i].second) {
      return false;
    }
  }
  return true;
}

bool SearchTest(SpatialTreeNode<int> *tree, int amount) {
  Point *possitions = new Point[amount];
  double gap = 1.0 / (amount + 1);
  std::minstd_rand simple_rand;
  simple_rand.seed(42);

  for (int i = 0; i < amount; i++) {
    possitions[i] = Point(gap * i, rand() / RAND_MAX, rand() / RAND_MAX);
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

    if (!IsEqual(manual_result, tree_search)) {
      delete[] possitions;
      return false;
    }
  }

  delete[] possitions;
  return true;
}

TEST(SpatialTreeTest, OctreeTest) {
  SpatialTreeNode<int> *tree =
      new OctreeNode<int>(Bound(0.0, 0.0, 0.0, 100.0, 100.0, 100.0), 100, 100);
  SimpleTest(tree);
  delete tree;
}

TEST(SpatialTreeTest, KdTest) {
  SpatialTreeNode<int> *tree =
      new KdTreeNode<int>(Bound(0.0, 0.0, 0.0, 100.0, 100.0, 100.0), 100, 100);
  SimpleTest(tree);
  delete tree;
}

TEST(SpatialTreeTest, OctreeSizeTest) {
  SpatialTreeNode<int> *tree =
      new OctreeNode<int>(Bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  SizeTest(tree, 1000);
  delete tree;
}

TEST(SpatialTreeTest, KdTreeSizeTest) {
  SpatialTreeNode<int> *tree =
      new KdTreeNode<int>(Bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  SizeTest(tree, 1000);
  delete tree;
}

TEST(SpatialTreeTest, OctreeSearchTest) {
  SpatialTreeNode<int> *tree =
      new OctreeNode<int>(Bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  ASSERT_TRUE(SearchTest(tree, 1000));
  delete tree;
}

TEST(SpatialTreeTest, KdTreeSearchTest) {
  SpatialTreeNode<int> *tree =
      new KdTreeNode<int>(Bound(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), 100, 100);
  ASSERT_TRUE(SearchTest(tree, 1000));
  delete tree;
}

}  // namespace bdm
