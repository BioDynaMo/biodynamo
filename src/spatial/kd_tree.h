#ifndef SPATIAL_KDTREE_H_
#define SPATIAL_KDTREE_H_

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <vector>
#include "spatial_tree.h"

namespace bdm {
/**
 * Next 3 vaiables are used for purposes of Surface Area Heuristics
 */
const int kNumerOfSpaces = 32;
const int kCt = 1;
const int kCi = 1;

using std::vector;
using std::pair;

/**
 * Spatial tree descendant.
 * Implements k-d tree. It organizes number of points in space with k
 * dimensions.
 * Provides different types of splitting space criterias
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
class KdTreeNode : public SpatialTreeNode<T> {
 public:
  KdTreeNode();

  KdTreeNode(Bound bnd, int max_depth, int max_amount_of_objects);

  KdTreeNode(Bound bnd, int max_depth, int max_amount_of_objects,
             int split);  // 0-mediana xyz, 1-mediana x, 2 - Center, 3-sah

  ~KdTreeNode();

  T At(Point p) const;

  int Size() const;

  virtual bool IsLeaf() const;

  virtual void Put(Point const &p, T obj);

  // little comparator for points
  static bool PointCompareX(const pair<Point, T> x, const pair<Point, T> y) {
    return (x.first.x < y.first.x);
  }

  static bool PointCompareY(const pair<Point, T> x, const pair<Point, T> y) {
    return (x.first.y < y.first.y);
  }

  static bool PointCompareZ(const pair<Point, T> x, const pair<Point, T> y) {
    return (x.first.z < y.first.z);
  }

 private:
  int axis_;
  Point median_;

  int split_parameter_;

  bool is_leaf_node;

  KdTreeNode<T> *children[2];

  vector<pair<Point, T>> *objects;

  int max_amount_of_objects_in_node;

  void SplitUsingVaryingMedian();  // splits on median_, changing axis_ every
  // function call in order: 0-x, 1-y, 2-z

  int GetChildID(Point p) const;

  virtual int GetChildrenSize() const;

  virtual SpatialTreeNode<T> **GetChildrenNodes() const;

  virtual vector<pair<Point, T>> *GetObjects() const;

  double node_area;

  int max_depth;

  Point GetMedian();

  double AreaOfKthPartOfSpaceNode(int k, int axis);

  Point GetSAHSplitPoint();

  Point GetMedianOnXAxis();

  void SplitUsingSingleXMedian();

  void SplitUsingSAH();

  void SplitUsingCenterOfSpaceNode();
};

/**
 * Empty constructor, initializes with bounds (0, 0, 0, 1, 1, 1),
 * maximum depth 10,
 * maximum amount of objects within 1 node 1000
 * @tparam T - type of the object to be stored in the tree
 */
template <typename T>
KdTreeNode<T>::KdTreeNode() {
  KdTreeNode<T>(Bound(0, 0, 0, 1, 1, 1), 10, 1000, 0);
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
KdTreeNode<T>::KdTreeNode(Bound bnd, int max_depth, int max_amount_of_objects) {
  this->bnd = bnd;
  this->is_leaf_node = true;
  this->max_depth = max_depth;
  this->max_amount_of_objects_in_node = max_amount_of_objects;
  for (int i = 0; i < 2; i++) children[i] = nullptr;
  objects = new vector<pair<Point, T>>();
  is_leaf_node = true;
  this->node_area = bnd.HalfSurfaceArea();
  this->axis_ = 0;
  this->split_parameter_ = 0;
}

/**
 * Constructor
 * @tparam T  - type of the object to be stored in the tree
 * @param bnd - Bound of the node
 * @param max_depth - maximum possible depth of the tree. After reaching that
 * point, nodes won't split
 * @param max_amount_of_objects - maximum number of object which can be stored
 * in 1 node.
 * @param split - indicates what type of split will we use to split the space
 * 0 - mediana split on varying axis, in order x, y, z;
 * 1 - mediana split on a single x axis;
 * 2 - center split on varying axis, in order x, y, z;
 * 3- split with surfce area heuristics(not useful at the moment, but may be
 * usful with bulk loading).
 */
template <typename T>
KdTreeNode<T>::KdTreeNode(Bound bnd, int max_depth, int max_amount_of_objects,
                          int split) {
  this->bnd = bnd;
  this->is_leaf_node = true;
  this->max_depth = max_depth;
  this->max_amount_of_objects_in_node = max_amount_of_objects;
  for (int i = 0; i < 2; i++) children[i] = nullptr;
  objects = new vector<pair<Point, T>>();
  is_leaf_node = true;
  this->node_area = bnd.HalfSurfaceArea();
  this->axis_ = 0;
  this->split_parameter_ = split;
}

/**
 * destructor
 * @tparam T
 */
template <typename T>
KdTreeNode<T>::~KdTreeNode() {
  for (int i = 0; i < 2; i++)
    if (children[i] != nullptr) {
      delete children[i];
      children[i] = nullptr;
    }

  if (is_leaf_node && objects != nullptr) {
    delete objects;
    objects = nullptr;
  }
}

/**
 * Adds new object to the tree
 * @param p - position of the new object
 * @param obj - object itself
 */
template <typename T>
void KdTreeNode<T>::Put(Point const &p, T obj) {
  // If a leaf node - Put there or SplitUsingVaryingMedian, else - proceed to
  // according child node
  if (is_leaf_node) {
    /** Put if maximum number of objects in a node don't exceed threshold
     * OR
     * if maximum depth was reached
     * ELSE
     * split node with, according to split parameter
     */

    if (objects->size() < max_amount_of_objects_in_node || (max_depth == 0)) {
      objects->push_back(make_pair(p, obj));

    } else {
      //  SplitUsingVaryingMedian
      switch (split_parameter_) {
        case 0:
          SplitUsingVaryingMedian();
          break;
        case 1:
          SplitUsingSingleXMedian();
          break;
        case 2:
          SplitUsingCenterOfSpaceNode();
          break;
        case 3:
          SplitUsingSAH();
          break;
        default:
          SplitUsingVaryingMedian();
          break;
      }
      Put(p, obj);
    }
  } else {
    int idx = GetChildID(p);
    children[idx]->Put(p, obj);
  }
}

/**
 * Split point differs each partition in order XYZ
 * Split point is median between all points on the axis
 */
template <typename T>
void KdTreeNode<T>::SplitUsingVaryingMedian() {
  double x_left, y_left, z_left, x_right, y_right, z_right;
  Bound bnd = this->bnd;
  if (!this->is_leaf_node) return;
  Point split_point = GetMedian();

  if (axis_ == 0) {
    x_left = split_point.x;
    y_left = bnd.nrt.y;
    z_left = bnd.nrt.z;
    x_right = split_point.x;
    y_right = bnd.flb.y;
    z_right = bnd.flb.z;
  } else if (axis_ == 1) {
    x_left = bnd.nrt.x;
    y_left = split_point.y;
    z_left = bnd.nrt.z;
    x_right = bnd.flb.x;
    y_right = split_point.y;
    z_right = bnd.flb.z;
  } else {
    x_left = bnd.nrt.x;
    y_left = bnd.nrt.y;
    z_left = split_point.z;
    x_right = bnd.flb.x;
    y_right = bnd.flb.y;
    z_right = split_point.z;
  }

  Point p[2] = {Point(bnd.flb.x, bnd.flb.y, bnd.flb.z),
                Point(x_right, y_right, z_right)};
  Point e[2] = {
      Point(x_left, y_left, z_left), Point(bnd.nrt.x, bnd.nrt.y, bnd.nrt.z)

  };

  for (int i = 0; i < 2; i++) {
    children[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth - 1,
                          max_amount_of_objects_in_node, split_parameter_);
    children[i]->axis_ = (axis_ + 1) % 3;
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
 * Split point only on X axis
 * Split point is median between all points on the axis
 */
template <typename T>
void KdTreeNode<T>::SplitUsingSingleXMedian() {
  Bound bnd = this->bnd;
  if (!this->is_leaf_node) return;
  Point split_point = GetMedianOnXAxis();

  Point p[2] = {
      Point(bnd.flb.x, bnd.flb.y, bnd.flb.z),
      Point(split_point.x, bnd.flb.y, bnd.flb.z),
  };
  Point e[2] = {Point(split_point.x, bnd.nrt.y, bnd.nrt.z),
                Point(bnd.nrt.x, bnd.nrt.y, bnd.nrt.z)

  };

  for (int i = 0; i < 2; i++) {
    children[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth - 1,
                          max_amount_of_objects_in_node, split_parameter_);
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
 * Split point calculated using surface area heuristics
 * Not useful at the moment as its main purpose is bulk loading
 */
template <typename T>
void KdTreeNode<T>::SplitUsingSAH() {
  double x_left, y_left, z_left, x_right, y_right, z_right;
  Bound bnd = this->bnd;
  if (!this->is_leaf_node) return;
  Point split_point = GetSAHSplitPoint();

  if (axis_ == 0) {
    x_left = split_point.x;
    y_left = bnd.nrt.y;
    z_left = bnd.nrt.z;
    x_right = split_point.x;
    y_right = bnd.flb.y;
    z_right = bnd.flb.z;
  } else if (axis_ == 1) {
    x_left = bnd.nrt.x;
    y_left = split_point.y;
    z_left = bnd.nrt.z;
    x_right = bnd.flb.x;
    y_right = split_point.y;
    z_right = bnd.flb.z;
  } else {
    x_left = bnd.nrt.x;
    y_left = bnd.nrt.y;
    z_left = split_point.z;
    x_right = bnd.flb.x;
    y_right = bnd.flb.y;
    z_right = split_point.z;
  }
  Point p[2] = {Point(bnd.flb.x, bnd.flb.y, bnd.flb.z),
                Point(x_right, y_right, z_right)};
  Point e[2] = {
      Point(x_left, y_left, z_left), Point(bnd.nrt.x, bnd.nrt.y, bnd.nrt.z)

  };

  for (int i = 0; i < 2; i++) {
    children[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth - 1,
                          max_amount_of_objects_in_node, split_parameter_);
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
 * Split point is center of box.
 * Coordinate differs each partition in order XYZ
 */
template <typename T>
void KdTreeNode<T>::SplitUsingCenterOfSpaceNode() {
  double x_left, y_left, z_left, x_right, y_right, z_right;
  Bound bnd = this->bnd;
  if (!this->is_leaf_node) return;

  if (axis_ == 0) {
    x_left = bnd.Center().x;
    y_left = bnd.nrt.y;
    z_left = bnd.nrt.z;
    x_right = bnd.Center().x;
    y_right = bnd.flb.y;
    z_right = bnd.flb.z;
  } else if (axis_ == 1) {
    x_left = bnd.nrt.x;
    y_left = bnd.Center().y;
    z_left = bnd.nrt.z;
    x_right = bnd.flb.x;
    y_right = bnd.Center().y;
    z_right = bnd.flb.z;
  } else {
    x_left = bnd.nrt.x;
    y_left = bnd.nrt.y;
    z_left = bnd.Center().z;
    x_right = bnd.flb.x;
    y_right = bnd.flb.y;
    z_right = bnd.Center().z;
  }

  Point p[2] = {Point(bnd.flb.x, bnd.flb.y, bnd.flb.z),
                Point(x_right, y_right, z_right)};
  Point e[2] = {
      Point(x_left, y_left, z_left), Point(bnd.nrt.x, bnd.nrt.y, bnd.nrt.z)

  };

  for (int i = 0; i < 2; i++) {
    children[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth - 1,
                          max_amount_of_objects_in_node, split_parameter_);
    children[i]->axis_ = (axis_ + 1) % 3;
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
 * Returns children nodes
 * @tparam T
 * @return
 */
template <typename T>
SpatialTreeNode<T> **KdTreeNode<T>::GetChildrenNodes() const {
  return (SpatialTreeNode<T> **)children;
}

template <typename T>
vector<pair<Point, T>> *KdTreeNode<T>::GetObjects() const {
  return this->objects;
}

template <typename T>
int KdTreeNode<T>::GetChildrenSize() const {
  if (!IsLeaf()) return 2;
  return 0;
}

template <typename T>
int KdTreeNode<T>::GetChildID(Point p) const {
  if (axis_ == 0) {
    if (p.x > this->median_.x)
      return 1;
    else
      return 0;
  } else if (axis_ == 1) {
    if (p.y > this->median_.y)
      return 1;
    else
      return 0;
  } else {
    if (p.z > this->median_.z)
      return 1;
    else
      return 0;
  }
}

template <typename T>
bool KdTreeNode<T>::IsLeaf() const {
  return is_leaf_node;
}

/**
 * Gets point, which we use for surface area heuristics
 * @param axis - on what axis are we separating: x=0,y=1,z=2
 * @param num - what parttion are we on (1;N)
 * @return sah rating
 */
template <typename T>
Point KdTreeNode<T>::GetSAHSplitPoint() {
  double objects_count[kNumerOfSpaces], leftside_count[kNumerOfSpaces - 1],
      rightside_count[kNumerOfSpaces - 1];
  double sah, temp;
  Point split_point;
  int axis = 0;

  Bound bnd = this->bnd;

  for (pair<Point, T> &i : *objects) {
    objects_count[(int)fmod(i.first.x, kNumerOfSpaces)];
  }

  for (int i = 0; i < kNumerOfSpaces; i++) {
    if (i == 0) {
      leftside_count[i] = objects_count[i];
      rightside_count[i] =
          std::accumulate(std::begin(objects_count), std::end(objects_count), 0,
                          std::plus<int>()) -
          objects_count[i];
    } else {
      leftside_count[i] = leftside_count[i - 1] + objects_count[i];
      rightside_count[i] = rightside_count[i - 1] - objects_count[i];
    }
  }

  sah = kCt +
        kCi * ((AreaOfKthPartOfSpaceNode(1, axis) * leftside_count[0] +
                AreaOfKthPartOfSpaceNode(31, axis)) /
               node_area);
  split_point.Set(bnd.flb.x + ((bnd.nrt.x - bnd.flb.x) * 1 / kNumerOfSpaces),
                  bnd.flb.y, bnd.flb.z);
  for (int i = 1; i < kNumerOfSpaces - 1; i++) {
    temp = kCt +
           kCi * ((AreaOfKthPartOfSpaceNode(i + 1, axis) * leftside_count[0] +
                   AreaOfKthPartOfSpaceNode(kNumerOfSpaces - i - 1, axis)) /
                  node_area);
    if (temp < sah) {
      sah = temp;
      split_point.Set(
          bnd.flb.x + ((bnd.nrt.x - bnd.flb.x) * (i + 1) / kNumerOfSpaces),
          bnd.flb.y, bnd.flb.z);
      this->axis_ = 0;
    }
  }

  axis = 1;

  for (pair<Point, T> &i : *objects) {
    objects_count[(int)fmod(i.first.y, kNumerOfSpaces)];
  }

  for (int i = 0; i < kNumerOfSpaces; i++) {
    if (i == 0) {
      leftside_count[i] = objects_count[i];
      rightside_count[i] =
          std::accumulate(std::begin(objects_count), std::end(objects_count), 0,
                          std::plus<int>()) -
          objects_count[i];
    } else {
      leftside_count[i] = leftside_count[i - 1] + objects_count[i];
      rightside_count[i] = rightside_count[i - 1] - objects_count[i];
    }
  }
  sah = kCt +
        kCi * ((AreaOfKthPartOfSpaceNode(1, axis) * leftside_count[0] +
                AreaOfKthPartOfSpaceNode(31, axis)) /
               node_area);
  split_point.Set(bnd.flb.x,
                  bnd.flb.y + ((bnd.nrt.y - bnd.flb.y) * 1 / kNumerOfSpaces),
                  bnd.flb.z);
  for (int i = 1; i < kNumerOfSpaces - 1; i++) {
    temp = kCt +
           kCi * ((AreaOfKthPartOfSpaceNode(i + 1, axis) * leftside_count[0] +
                   AreaOfKthPartOfSpaceNode(kNumerOfSpaces - i - 1, axis)) /
                  node_area);
    if (temp < sah) {
      sah = temp;
      split_point.Set(bnd.flb.x, bnd.flb.y + ((bnd.nrt.y - bnd.flb.y) *
                                              (i + 1) / kNumerOfSpaces),
                      bnd.flb.z);
      this->axis_ = 1;
    }
  }

  axis = 2;
  for (pair<Point, T> &i : *objects) {
    objects_count[(int)fmod(i.first.z, kNumerOfSpaces)];
  }

  for (int i = 0; i < kNumerOfSpaces; i++) {
    if (i == 0) {
      leftside_count[i] = objects_count[i];
      rightside_count[i] =
          std::accumulate(std::begin(objects_count), std::end(objects_count), 0,
                          std::plus<int>()) -
          objects_count[i];
    } else {
      leftside_count[i] = leftside_count[i - 1] + objects_count[i];
      rightside_count[i] = rightside_count[i - 1] - objects_count[i];
    }
  }
  sah = kCt +
        kCi * ((AreaOfKthPartOfSpaceNode(1, axis) * leftside_count[0] +
                AreaOfKthPartOfSpaceNode(31, axis)) /
               node_area);
  split_point.Set(bnd.flb.x, bnd.flb.y,
                  bnd.flb.z + ((bnd.nrt.z - bnd.flb.z) * 1 / kNumerOfSpaces));
  for (int i = 1; i < kNumerOfSpaces - 1; i++) {
    temp = kCt +
           kCi * ((AreaOfKthPartOfSpaceNode(i + 1, axis) * leftside_count[0] +
                   AreaOfKthPartOfSpaceNode(kNumerOfSpaces - i - 1, axis)) /
                  node_area);
    if (temp < sah) {
      sah = temp;
      split_point.Set(
          bnd.flb.x, bnd.flb.y,
          bnd.flb.z + ((bnd.nrt.z - bnd.flb.z) * (i + 1) / kNumerOfSpaces));
      this->axis_ = 2;
    }
  }

  return split_point;
}

/**
 * Calculates median point for a certain axis
 * @return median on certain axis
 */
template <typename T>
Point KdTreeNode<T>::GetMedian() {
  vector<pair<Point, T>> *sorted = this->objects;
  if (axis_ == 0) {
    // std::sort(sorted->begin(), sorted->end(), PointCompareX);
    std::nth_element(sorted->begin(), sorted->begin() + sorted->size() / 2,
                     sorted->end(), PointCompareX);

  } else if (axis_ == 1) {
    // std::sort(sorted->begin(), sorted->end(), PointCompareY);
    std::nth_element(sorted->begin(), sorted->begin() + sorted->size() / 2,
                     sorted->end(), PointCompareY);
  } else {
    // std::sort(sorted->begin(), sorted->end(), PointCompareZ);
    std::nth_element(sorted->begin(), sorted->begin() + sorted->size() / 2,
                     sorted->end(), PointCompareZ);
  }
  this->median_ = sorted->at(sorted->size() / 2).first;
  return sorted->at(sorted->size() / 2).first;
}

template <typename T>
Point KdTreeNode<T>::GetMedianOnXAxis() {
  vector<pair<Point, T>> *sorted = this->objects;

  //  std::sort(sorted->begin(), sorted->end(), PointCompareX);
  std::nth_element(sorted->begin(), sorted->begin() + sorted->size() / 2,
                   sorted->end(), PointCompareX);
  this->median_ = sorted->at(sorted->size() / 2).first;
  return sorted->at(sorted->size() / 2).first;
}

/**
 * Returns area of the surface, obtained after splitting original space node on
 * a certain axis
 * @tparam T - type of the object
 * @param k - part of the surface, which is wanted to be found
 * @param axis - what axis should be used to split
 * @return - area value
 */
template <typename T>
double KdTreeNode<T>::AreaOfKthPartOfSpaceNode(int k, int axis) {
  if (axis == 0) {
    return this->node_area -
           k / kNumerOfSpaces * (this->bnd.Height() * this->bnd.Length()) -
           k / kNumerOfSpaces * (this->bnd.Width() * this->bnd.Length());
  } else if (axis == 1) {
    return this->node_area -
           k / kNumerOfSpaces * (this->bnd.Height() * this->bnd.Width()) -
           k / kNumerOfSpaces * (this->bnd.Width() * this->bnd.Length());
  } else if (axis == 2) {
    return this->node_area -
           (k / kNumerOfSpaces * (this->bnd.Height() * this->bnd.Width()) +
            k / kNumerOfSpaces * (this->bnd.Height() * this->bnd.Length()));
  } else
    return -1;
}
}

#endif  //  SPATIAL_KDTREE_H_