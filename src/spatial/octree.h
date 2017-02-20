#ifndef SPATIAL_OCTREE_H_
#define SPATIAL_OCTREE_H_

#include <cstdio>
#include <vector>

#include "spatial_tree.h"

namespace bdm {

using std::vector;
using std::make_pair;

/**
 * Spatial tree descendant.
 * Implements octree. It organizes number of points in space with 8 dimensions.
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
class OctreeNode : public SpatialTreeNode<T> {
 public:
  OctreeNode();

  OctreeNode(Bound bnd, int max_depth, int max_amount_of_objects);

  OctreeNode(int max_depth, int max_amount_of_objects);

  ~OctreeNode();

  virtual bool IsLeaf() const;

  virtual void Put(Point const &p, T obj);

  T At(Point const &p) const;

  int Size() const;

 private:
  bool is_leaf_node;
  OctreeNode<T> *children[8];
  vector<pair<Point, T> > *objects;
  int max_depth;
  int max_amount_of_objects_in_node;

  void Split();

  int GetChildID(Point const &p) const;

  virtual SpatialTreeNode<T> **GetChildrenNodes() const;

  virtual vector<pair<Point, T> > *GetObjects() const;

  virtual int GetChildrenSize() const;
};

/**
 * Empty constructor, initializes with bounds (0, 0, 0, 1, 1, 1),
 * maximum depth 10,
 * maximum amount of objects within 1 node 1000
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
OctreeNode<T>::OctreeNode() {
  OctreeNode<T>(Bound(0, 0, 0, 1, 1, 1), 10, 1000);
}

/**
 * Constructor
 * @tparam T  - type of the object to be stored in the tree
 * @param max_depth - maximum possible depth of the tree. After reaching that
 * point, nodes won't split
 * @param max_amount_of_objects - maximum number of object which can be stored
 * in 1 node.
 * In our case, amount of object acts as splitting criteria
 */
template <typename T>
OctreeNode<T>::OctreeNode(int max_depth, int max_amount_of_objects) {
  this->bnd = Bound(-inf, -inf, -inf, inf, inf, inf);
  this->is_leaf_node = true;
  this->max_depth = max_depth;
  this->max_amount_of_objects_in_node = max_amount_of_objects;
  for (int i = 0; i < 8; i++) children[i] = nullptr;
  objects = new vector<pair<Point, T> >();
  is_leaf_node = true;
}

/**
 * Constructor
 * @tparam T  - type of the object to be stored in the tree
 * @param bnd - Bound of the node
 * @param max_depth - maximum possible depth of the tree. After reaching that
 * point, nodes won't split
 * @param max_amount_of_objects - maximum number of object which can be stored
 * in 1 node.
 * In our case, amount of object acts as splitting criteria
 */
template <typename T>
OctreeNode<T>::OctreeNode(Bound bnd, int max_depth, int max_amount_of_objects) {
  this->bnd = bnd;
  this->is_leaf_node = true;
  this->max_depth = max_depth;
  this->max_amount_of_objects_in_node = max_amount_of_objects;
  for (int i = 0; i < 8; i++) children[i] = nullptr;
  objects = new vector<pair<Point, T> >();
  is_leaf_node = true;
}

/**
 * destructor
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
OctreeNode<T>::~OctreeNode() {
  if (!IsLeaf()) {
    for (int i = 0; i < 8; i++)
      if (children[i] != nullptr) {
        delete children[i];
        children[i] = nullptr;
      }
  }
  if (objects != nullptr) {
    delete objects;
    objects = nullptr;
  }
}

template <typename T>
bool OctreeNode<T>::IsLeaf() const {
  return is_leaf_node;
}

/**
 * Adds new object to the tree
 * @param p - position of the new object
 * @param obj - object itself
 */
template <typename T>
void OctreeNode<T>::Put(Point const &p, T obj) {
  if (is_leaf_node) {
    // Insert
    if (objects->size() < max_amount_of_objects_in_node || (max_depth == 0)) {
      objects->push_back(make_pair(p, obj));
    } else {
      // SplitUsingVaryingMedian
      Split();
      Put(p, obj);
    }
  } else {
    int idx = GetChildID(p);
    children[idx]->Put(p, obj);
  }
}

/**
 * Splits node to 8 equal subspaces
 * @tparam T - type of objects
 */
template <typename T>
void OctreeNode<T>::Split() {
  if (!this->is_leaf_node) return;
  Point center = this->bnd.Center();
  Bound bnd = this->bnd;
  Point p[8] = {Point(center.x, center.y, center.z),
                Point(center.x, bnd.Left(), center.z),
                Point(center.x, bnd.Left(), bnd.Bottom()),
                Point(center.x, center.y, bnd.Bottom()),
                Point(bnd.Far(), center.y, center.z),
                Point(bnd.Far(), bnd.Left(), center.z),
                Point(bnd.Far(), bnd.Left(), bnd.Bottom()),
                Point(bnd.Far(), center.y, bnd.Bottom())};
  Point e[8] = {Point(bnd.Near(), bnd.Right(), bnd.Top()),
                Point(bnd.Near(), center.y, bnd.Top()),
                Point(bnd.Near(), center.y, center.z),
                Point(bnd.Near(), bnd.Right(), center.z),
                Point(center.x, bnd.Right(), bnd.Top()),
                Point(center.x, center.y, bnd.Top()),
                Point(center.x, center.y, center.z),
                Point(center.x, bnd.Right(), center.z)};

  for (int i = 0; i < 8; i++) {
    children[i] = new OctreeNode<T>(Bound(p[i], e[i]), max_depth - 1,
                                    max_amount_of_objects_in_node);
  }

  for (int i = 0; i < objects->size(); i++) {
    int idx = GetChildID(objects->at(i).first);
    children[idx]->Put(objects->at(i).first, objects->at(i).second);
  }
  delete objects;
  objects = nullptr;
  this->is_leaf_node = false;
}

/**
 * returns in what octa subspace point p is located
 * @tparam T - type of the object
 * @param p - point in space
 * @return number of subspace
 */
template <typename T>
int OctreeNode<T>::GetChildID(Point const &p) const {
  Point center = this->bnd.Center();
  int result = 0;
  if (p.y >= center.y) {
    if (p.z >= center.z) {
      result = 0;
    } else {
      result = 3;
    }
  } else {
    if (p.z >= center.z) {
      result = 1;
    } else {
      result = 2;
    }
  }
  if (p.x < center.x) {
    result += 4;
  }
  return result;
}

template <typename T>
T OctreeNode<T>::At(Point const &p) const {
  if (this->is_leaf_node) {
    for (int i = 0; i < objects->size(); i++)
      if (p.equals(objects->at(i).first)) return objects->at(i).second;
  } else {
    int idx = GetChildID(p);
    return children[idx]->At(p);
  }
  return NULL;
}

template <typename T>
int OctreeNode<T>::Size() const {
  if (this->is_leaf_node) {
    return static_cast<int>(objects->size());
  } else {
    int sum = 0;
    for (int i = 0; i < 8; i++) {
      sum += children[i]->Size();
    }
    return sum;
  }
}

template <typename T>
SpatialTreeNode<T> **OctreeNode<T>::GetChildrenNodes() const {
  return (SpatialTreeNode<T> **)children;
}

template <typename T>
vector<pair<Point, T> > *OctreeNode<T>::GetObjects() const {
  return this->objects;
}

template <typename T>
int OctreeNode<T>::GetChildrenSize() const {
  if (!IsLeaf()) return 8;
  return 0;
}
}
#endif /* SPATIAL_OCTREE_H_ */