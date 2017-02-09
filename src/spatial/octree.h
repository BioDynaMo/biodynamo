#ifndef SRC_SPATIAL_OCTREE_H_
#define SRC_SPATIAL_OCTREE_H_

#include <cstdio>
#include <vector>

#include "spatial_tree.h"

using std::vector;
using std::make_pair;

/**
 * Spatial tree descendant.
 * Implements octree. It organizes number of points in space with 8 dimensions.
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
class octree_node : public spatial_tree_node<T> {
 private:
  bool is_leaf_node;
  octree_node<T> *children[8];
  vector<pair<point, T> > *objects;
  int max_depth;
  int max_amount_of_objects_in_node;

  void Split();
  int GetChildID(point p);

  virtual spatial_tree_node<T> **GetChildrenNodes();
  virtual vector<pair<point, T> > *GetObjects();
  virtual int GetChildrenSize();

 public:
  octree_node();
  octree_node(bound bnd, int max_depth, int max_amount_of_objects);
  octree_node(int max_depth, int max_amount_of_objects);
  ~octree_node();
  virtual bool IsLeaf();
  virtual void Put(point p, T obj);
  T At(point p);
  int Size();
};

/**
 * Empty constructor, initializes with bounds (0, 0, 0, 1, 1, 1),
 * maximum depth 10,
 * maximum amount of objects within 1 node 1000
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
octree_node<T>::octree_node() {
  octree_node<T>(bound(0, 0, 0, 1, 1, 1), 10, 1000);
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
octree_node<T>::octree_node(int max_depth, int max_amount_of_objects) {
  this->bnd = bound(-inf, -inf, -inf, inf, inf, inf);
  this->is_leaf_node = true;
  this->max_depth = max_depth;
  this->max_amount_of_objects_in_node = max_amount_of_objects;
  for (int i = 0; i < 8; i++) children[i] = nullptr;
  objects = new vector<pair<point, T> >();
  is_leaf_node = true;
}

/**
 * Constructor
 * @tparam T  - type of the object to be stored in the tree
 * @param bnd - bound of the node
 * @param max_depth - maximum possible depth of the tree. After reaching that
 * point, nodes won't split
 * @param max_amount_of_objects - maximum number of object which can be stored
 * in 1 node.
 * In our case, amount of object acts as splitting criteria
 */
template <typename T>
octree_node<T>::octree_node(bound bnd, int max_depth,
                            int max_amount_of_objects) {
  this->bnd = bnd;
  this->is_leaf_node = true;
  this->max_depth = max_depth;
  this->max_amount_of_objects_in_node = max_amount_of_objects;
  for (int i = 0; i < 8; i++) children[i] = nullptr;
  objects = new vector<pair<point, T> >();
  is_leaf_node = true;
}

/**
 * destructor
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
octree_node<T>::~octree_node() {
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
bool octree_node<T>::IsLeaf() {
  return is_leaf_node;
}

/**
 * Adds new object to the tree
 * @param p - position of the new object
 * @param obj - object itself
 */
template <typename T>
void octree_node<T>::Put(point p, T obj) {
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
void octree_node<T>::Split() {
  if (!this->is_leaf_node) return;
  point center = this->bnd.Center();
  bound bnd = this->bnd;
  point p[8] = {point(center.x, center.y, center.z),
                point(center.x, bnd.Left(), center.z),
                point(center.x, bnd.Left(), bnd.Bottom()),
                point(center.x, center.y, bnd.Bottom()),
                point(bnd.Far(), center.y, center.z),
                point(bnd.Far(), bnd.Left(), center.z),
                point(bnd.Far(), bnd.Left(), bnd.Bottom()),
                point(bnd.Far(), center.y, bnd.Bottom())};
  point e[8] = {point(bnd.Near(), bnd.Right(), bnd.Top()),
                point(bnd.Near(), center.y, bnd.Top()),
                point(bnd.Near(), center.y, center.z),
                point(bnd.Near(), bnd.Right(), center.z),
                point(center.x, bnd.Right(), bnd.Top()),
                point(center.x, center.y, bnd.Top()),
                point(center.x, center.y, center.z),
                point(center.x, bnd.Right(), center.z)};

  for (int i = 0; i < 8; i++) {
    children[i] = new octree_node<T>(bound(p[i], e[i]), max_depth - 1,
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
int octree_node<T>::GetChildID(point p) {
  point center = this->bnd.Center();
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
T octree_node<T>::At(point p) {
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
int octree_node<T>::Size() {
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
spatial_tree_node<T> **octree_node<T>::GetChildrenNodes() {
  return (spatial_tree_node<T> **)children;
}

template <typename T>
vector<pair<point, T> > *octree_node<T>::GetObjects() {
  return this->objects;
}

template <typename T>
int octree_node<T>::GetChildrenSize() {
  if (!IsLeaf()) return 8;
  return 0;
}

#endif /* SRC_SPATIAL_OCTREE_H_ */