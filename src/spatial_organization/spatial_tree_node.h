#ifndef SPATIAL_ORGANIZATION_SPATIAL_TREE_NODE_H_
#define SPATIAL_ORGANIZATION_SPATIAL_TREE_NODE_H_

#include <utility>
#include <vector>
#include "spatial_organization/bound.h"

namespace bdm {
namespace spatial_organization {

using std::vector;
using std::pair;

/// Is a core class, which provides with variety of virtual
/// functions for children-type trees and an implemented search neighbours
/// function,
/// which is common for all the trees.
template <typename T>
class SpatialTreeNode {
 public:
  SpatialTreeNode() {}

  virtual ~SpatialTreeNode() {}

  ///  @tparam T - type of the object
  ///  @return bounds of the node
  Bound GetBound() const;

  virtual bool IsLeaf() const = 0;

  virtual void Put(Point const &p, T obj) = 0;

  ///  @tparam T - type of the object
  ///  @param distance - distance to search within
  ///  @return
  virtual vector<pair<T, T> > GetNeighbors(double distance) const;

 protected:
  Bound bound_;

 private:
  virtual SpatialTreeNode<T> **GetChildrenNodes() const = 0;

  virtual int GetChildrenSize() const = 0;

  virtual vector<pair<Point, T> > *GetObjects() const = 0;

  ///  Neighbour search function
  ///  Search pairs (a, b) where a from A and b from B
  ///  Absolutelly the same method. The only difference is result type
  ///  @tparam T
  ///  @param A - node of the tree
  ///  @param B - node of the tree
  ///  @param distance - finding neighbors within that distance
  ///  @param result - container, where we save retrieved data (all neighbor pairs
  ///  without its points)
  static void GetNeighbors(SpatialTreeNode<T> const *A,
                           SpatialTreeNode<T> const *B, double distance,
                           vector<pair<T, T> > *result);
};

template <typename T>
Bound SpatialTreeNode<T>::GetBound() const {
  return bound_;
}

template <typename T>
vector<pair<T, T> > SpatialTreeNode<T>::GetNeighbors(double distance) const {
  vector<pair<T, T> > result;
  GetNeighbors(this, this, distance * distance, &result);
  return result;
}

template <typename T>
void SpatialTreeNode<T>::GetNeighbors(SpatialTreeNode<T> const *A,
                                      SpatialTreeNode<T> const *B,
                                      double distance,
                                      vector<pair<T, T> > *result) {
  if (A->IsLeaf() && B->IsLeaf()) {
    auto a_objs = A->GetObjects();
    auto b_objs = B->GetObjects();

    bool is_same = A == B;

    for (int i = 0; i < a_objs->size(); i++)
      for (int j = is_same ? (i + 1) : 0; j < b_objs->size(); j++)
        if ((a_objs->at(i).first)
                .SquaredEuclidianDistance(b_objs->at(j).first) <= distance)
          result->push_back(
              make_pair(a_objs->at(i).second, b_objs->at(j).second));
  } else {
    SpatialTreeNode<T> **a_nodes, **b_nodes;
    int a_size = 0, b_size = 0;
    if (A->IsLeaf()) {
      b_nodes = B->GetChildrenNodes();
      for (int i = 0; i < B->GetChildrenSize(); i++)
        if (A->GetBound().SquaredDistance(b_nodes[i]->GetBound()) <= distance)
          GetNeighbors(A, b_nodes[i], distance, result);
    } else if (B->IsLeaf()) {
      a_nodes = A->GetChildrenNodes();
      for (int i = 0; i < A->GetChildrenSize(); i++)
        if (a_nodes[i]->GetBound().SquaredDistance(B->GetBound()) <= distance)
          GetNeighbors(a_nodes[i], B, distance, result);
    } else {
      a_nodes = A->GetChildrenNodes();
      b_nodes = B->GetChildrenNodes();
      a_size = A->GetChildrenSize();
      b_size = B->GetChildrenSize();
      bool is_same = A == B;
      for (int i = 0; i < a_size; i++)
        for (int j = is_same ? (i) : 0; j < b_size; j++)
          if (a_nodes[i]->GetBound().SquaredDistance(b_nodes[j]->GetBound()) <=
              distance)
            GetNeighbors(a_nodes[i], b_nodes[j], distance, result);
    }
  }
}
}  // namespace spatial_organization
}  // namespace bdm
#endif  // SPATIAL_ORGANIZATION_SPATIAL_TREE_NODE_H_
