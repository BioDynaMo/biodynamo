/**
 * Is a core class, which provides with variety of virtual
 * functions for children-type trees and an implemented search neighbours
 * function,
 * which is common for all the trees.
 */
#ifndef SRC_SPATIAL_SPATIAL_TREE_H_
#define SRC_SPATIAL_SPATIAL_TREE_H_

#include <stdio.h>
#include <utility>
#include <vector>
#include "bound.h"

using std::vector;
using std::pair;

template <typename T>
class spatial_tree_node {
 protected:
  bound bnd;

 private:
  virtual spatial_tree_node<T> **GetChildrenNodes() = 0;

  virtual int GetChildrenSize() = 0;

  virtual vector<pair<point, T> > *GetObjects() = 0;

  static void GetNeighbors(
          spatial_tree_node<T> *A, spatial_tree_node<T> *B, double distance,
          vector<pair<pair<point, T>, pair<point, T> > > *result);

  static void GetNeighbors(spatial_tree_node<T> *A, spatial_tree_node<T> *B,
                           double distance, vector<pair<T, T> > *result);

 public:
  spatial_tree_node() {}

  virtual ~spatial_tree_node() {}

  bound GetBound();

  virtual bool IsLeaf() = 0;

  virtual void Put(point p, T obj) = 0;

  virtual vector<pair<pair<point, T>, pair<point, T> > >
  GetNeighborsWithPoints(double distance);

  virtual vector<pair<T, T> > GetNeighbors(double distance);
};

/**
 *
 * @tparam T - type of the object
 * @return bounds of the node
 */
template <typename T>
bound spatial_tree_node<T>::GetBound() {
  return bnd;
}

template <typename T>
vector<pair<pair<point, T>, pair<point, T> > >
spatial_tree_node<T>::GetNeighborsWithPoints(double distance) {
  vector<pair<pair<point, T>, pair<point, T> > > result;
    GetNeighbors(this, this, distance * distance, &result);
  return result;
}

/**
 *
 * @tparam T - type of the object
 * @param distance - distance to search within
 * @return
 */
template <typename T>
vector<pair<T, T> > spatial_tree_node<T>::GetNeighbors(double distance) {
  vector<pair<T, T> > result;
    GetNeighbors(this, this, distance * distance, &result);
  return result;
}

/**
 * Makes pair, using points and objects
 * @tparam T - type of the object
 * @param p1 - point, indicates location of 1st object
 * @param o1 - 1st object
 * @param p2 - point, indicates location of 2nd object
 * @param o2 - 2nd object
 * @return pair of neighbors
 */
template <typename T>
pair<pair<point, T>, pair<point, T> > MakeNeighborPair(point p1, T o1,
                                                       point p2, T o2) {
  return make_pair(make_pair(p1, o1), make_pair(p2, o2));
}

/**
 *
 * @tparam T - type of the object
 * @param p1 - 1st object pair(location point, object)
 * @param p2 - 2nd object pair(location point, object)
 * @return pair of neighbors
 */
template <typename T>
pair<pair<point, T>, pair<point, T> > MakeNeighborPair(pair<point, T> p1,
                                                       pair<point, T> p2) {
  return make_pair(make_pair(p1.first, p1.second),
                   make_pair(p2.first, p2.second));
}

/**
 * Neighbour search function
 * Search pairs (a, b) where a from A and b from B
 * @tparam T
 * @param A - node of the tree
 * @param B - node of the tree
 * @param distance - finding neighbors within that distance
 * @param result - container, where we save retrieved data (all neighbor pairs
 * with its points)
 */
template <typename T>
void spatial_tree_node<T>::GetNeighbors(
        spatial_tree_node<T> *A, spatial_tree_node<T> *B, double distance,
        vector<pair<pair<point, T>, pair<point, T> > > *result) {
  // Two cases:
  //   A and B are leaf nodes - check every pair
  //   otherwise do recoursive call on children
  if (A->IsLeaf() && B->IsLeaf()) {
    auto a_objs = A->GetObjects();
    auto b_objs = B->GetObjects();

    bool is_same = A == B;

    for (int i = 0; i < a_objs->size(); i++)
      for (int j = is_same ? (i + 1) : 0; j < b_objs->size(); j++)
        if ((a_objs->at(i).first).SquaredEuclidianDistance(b_objs->at(j).first) <= distance)
          result->push_back(MakeNeighborPair(a_objs->at(i), b_objs->at(j)));
  } else {
    spatial_tree_node<T> **a_nodes, **b_nodes;
    int a_size = 0, b_size = 0;
    // If A is a leaf node than call with A and child of B
    if (A->IsLeaf()) {
      b_nodes = B->GetChildrenNodes();
      for (int i = 0; i < B->GetChildrenSize(); i++)
        if (A->GetBound().Distance(b_nodes[i]->GetBound()) <= distance)
            GetNeighbors(A, b_nodes[i], distance, result);
    } else if (B->IsLeaf()) {
      a_nodes = A->GetChildrenNodes();
      for (int i = 0; i < A->GetChildrenSize(); i++)
        if (a_nodes[i]->GetBound().Distance(B->GetBound()) <= distance)
            GetNeighbors(a_nodes[i], B, distance, result);
    } else {
      a_nodes = A->GetChildrenNodes();
      b_nodes = B->GetChildrenNodes();
      a_size = A->GetChildrenSize();
      b_size = B->GetChildrenSize();
      bool is_same = A == B;
      for (int i = 0; i < a_size; i++)
        for (int j = is_same ? (i) : 0; j < b_size; j++)
          if (a_nodes[i]->GetBound().Distance(b_nodes[j]->GetBound()) <=
              distance)
              GetNeighbors(a_nodes[i], b_nodes[j], distance, result);
    }
  }
}

/**
 * Neighbour search function
 * Search pairs (a, b) where a from A and b from B
 * Absolutelly the same method. The only difference is result type
 * @tparam T
 * @param A - node of the tree
 * @param B - node of the tree
 * @param distance - finding neighbors within that distance
 * @param result - container, where we save retrieved data (all neighbor pairs
 * without its points)
 */
template <typename T>
void spatial_tree_node<T>::GetNeighbors(spatial_tree_node<T> *A,
                                        spatial_tree_node<T> *B,
                                        double distance,
                                        vector<pair<T, T> > *result) {
  if (A->IsLeaf() && B->IsLeaf()) {
    auto a_objs = A->GetObjects();
    auto b_objs = B->GetObjects();

    bool is_same = A == B;

    for (int i = 0; i < a_objs->size(); i++)
      for (int j = is_same ? (i + 1) : 0; j < b_objs->size(); j++)
        if ((a_objs->at(i).first).SquaredEuclidianDistance(b_objs->at(j).first) <= distance)
          result->push_back(
              make_pair(a_objs->at(i).second, b_objs->at(j).second));
  } else {
    spatial_tree_node<T> **a_nodes, **b_nodes;
    int a_size = 0, b_size = 0;
    if (A->IsLeaf()) {
      b_nodes = B->GetChildrenNodes();
      for (int i = 0; i < B->GetChildrenSize(); i++)
        if (A->GetBound().Distance(b_nodes[i]->GetBound()) <= distance)
            GetNeighbors(A, b_nodes[i], distance, result);
    } else if (B->IsLeaf()) {
      a_nodes = A->GetChildrenNodes();
      for (int i = 0; i < A->GetChildrenSize(); i++)
        if (a_nodes[i]->GetBound().Distance(B->GetBound()) <= distance)
            GetNeighbors(a_nodes[i], B, distance, result);
    } else {
      a_nodes = A->GetChildrenNodes();
      b_nodes = B->GetChildrenNodes();
      a_size = A->GetChildrenSize();
      b_size = B->GetChildrenSize();
      bool is_same = A == B;
      for (int i = 0; i < a_size; i++)
        for (int j = is_same ? (i) : 0; j < b_size; j++)
          if (a_nodes[i]->GetBound().Distance(b_nodes[j]->GetBound()) <=
              distance)
              GetNeighbors(a_nodes[i], b_nodes[j], distance, result);
    }
  }
}

#endif /* SRC_SPATIAL_SPATIAL_TREE_H_ */