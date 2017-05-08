#ifndef SPATIAL_ORGANIZATION_KD_TREE_NODE_H_
#define SPATIAL_ORGANIZATION_KD_TREE_NODE_H_

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>
#include "spatial_organization/spatial_tree_node.h"

namespace bdm {
namespace spatial_organization {

/// Next 3 vaiables are used for purposes of Surface Area Heuristics
const int kNumerOfSpaces = 32;
const int kCt = 1;
const int kCi = 1;

using std::vector;
using std::pair;

/// Spatial tree descendant.
/// Implements k-d tree. It organizes number of points in space with k
/// dimensions.
/// Provides different types of splitting space criterias
/// @tparam T - type of the object to be stored in the tree
template <typename T>
class KdTreeNode : public SpatialTreeNode<T> {
 public:
  /// Empty constructor, initializes with bounds (0, 0, 0, 1, 1, 1),
  /// maximum depth 10,
  /// maximum amount of objects within 1 node 1000
  /// @tparam T - type of the object to be stored in the tree
  KdTreeNode();

  /// Constructor
  /// @tparam T  - type of the object to be stored in the tree
  /// @param bnd - Bound of the node
  /// @param max_depth - maximum possible depth of the tree. After reaching that
  /// point, nodes won't split
  /// @param max_amount_of_objects - maximum number of object which can be
  /// stored
  /// in 1 node.
  /// In our case, amount of object acts as splitting criteria
  KdTreeNode(const Bound &bnd, int max_depth, int max_amount_of_objects);

  /// Constructor
  /// @tparam T  - type of the object to be stored in the tree
  /// @param bnd - Bound of the node
  /// @param max_depth - maximum possible depth of the tree. After reaching that
  /// point, nodes won't split
  /// @param max_amount_of_objects - maximum number of object which can be
  /// stored
  /// in 1 node.
  /// @param split - indicates what type of split will we use to split the space
  /// 0 - mediana split on varying axis, in order x, y, z;
  /// 1 - mediana split on a single x axis;
  /// 2 - center split on varying axis, in order x, y, z;
  /// 3- split with surfce area heuristics(not useful at the moment, but may be
  /// usful with bulk loading).
  KdTreeNode(const Bound &bnd, int max_depth, int max_amount_of_objects,
             int split);  // 0-mediana xyz, 1-mediana x, 2 - Center, 3-sah

  /// destructor
  /// @tparam T
  ~KdTreeNode();

  T At(Point p) const;

  size_t Size() const;

  bool IsLeaf() const override;

  /// Adds new object to the tree
  /// @param p - position of the new object
  /// @param obj - object itself
  void Put(Point const &p, T obj) override;

  /// little comparator for points
  static bool PointCompareX(const pair<Point, T> x, const pair<Point, T> y) {
    return (x.first.x_ < y.first.x_);
  }

  static bool PointCompareY(const pair<Point, T> x, const pair<Point, T> y) {
    return (x.first.y_ < y.first.y_);
  }

  static bool PointCompareZ(const pair<Point, T> x, const pair<Point, T> y) {
    return (x.first.z_ < y.first.z_);
  }

 private:
  int axis_;
  Point median_;
  int split_parameter_;
  bool is_leaf_node_;
  KdTreeNode<T> *children_[2];
  vector<pair<Point, T>> objects_;
  size_t max_amount_of_objects_in_node_;
  double node_area_;
  size_t max_depth_;

  /// Split point differs each partition in order XYZ
  /// Split point is median between all points on the axis
  void SplitUsingVaryingMedian();  // splits on median_, changing axis_ every
  // function call in order: 0-x, 1-y, 2-z

  int GetChildID(Point p) const;

  size_t GetChildrenSize() const override;

  /// Returns children nodes
  /// @tparam T
  /// @return
  SpatialTreeNode<T> **GetChildrenNodes() const override;

  const vector<pair<Point, T>> &GetObjects() const override;

  /// Calculates median point for a certain axis
  /// @return median on certain axis
  Point GetMedian();

  /// Returns area of the surface, obtained after splitting original space node
  /// on a certain axis
  /// @tparam T - type of the object
  /// @param k - part of the surface, which is wanted to be found
  /// @param axis - what axis should be used to split
  /// @return - area value
  double AreaOfKthPartOfSpaceNode(int k, int axis);

  /// Gets point, which we use for surface area heuristics
  /// @param axis - on what axis are we separating: x=0,y=1,z=2
  /// @param num - what parttion are we on (1;N)
  /// @return sah rating
  Point GetSAHSplitPoint();

  Point GetMedianOnXAxis();

  /// Split point only on X axis
  /// Split point is median between all points on the axis
  void SplitUsingSingleXMedian();

  /// Split point calculated using surface area heuristics
  /// Not useful at the moment as its main purpose is bulk loading
  void SplitUsingSAH();

  /// Split point is center of box.
  /// Coordinate differs each partition in order XYZ
  void SplitUsingCenterOfSpaceNode();
};

template <typename T>
KdTreeNode<T>::KdTreeNode() {
  KdTreeNode<T>(Bound(0, 0, 0, 1, 1, 1), 10, 1000, 0);
}

template <typename T>
KdTreeNode<T>::KdTreeNode(const Bound &bnd, int max_depth,
                          int max_amount_of_objects)
    : SpatialTreeNode<T>(bnd),
      axis_(0),
      split_parameter_(0),
      is_leaf_node_(true),
      max_amount_of_objects_in_node_(max_amount_of_objects),
      node_area_(bnd.HalfSurfaceArea()),
      max_depth_(max_depth) {
  for (int i = 0; i < 2; i++) {
    children_[i] = nullptr;
  }
}

template <typename T>
KdTreeNode<T>::KdTreeNode(const Bound &bnd, int max_depth,
                          int max_amount_of_objects, int split)
    : SpatialTreeNode<T>(bnd),
      axis_(0),
      split_parameter_(split),
      is_leaf_node_(true),
      max_amount_of_objects_in_node_(max_amount_of_objects),
      node_area_(bnd.HalfSurfaceArea()),
      max_depth_(max_depth) {
  for (int i = 0; i < 2; i++) {
    children_[i] = nullptr;
  }
}

template <typename T>
KdTreeNode<T>::~KdTreeNode() {
  for (int i = 0; i < 2; i++) {
    if (children_[i] != nullptr) {
      delete children_[i];
      children_[i] = nullptr;
    }
  }
}

template <typename T>
void KdTreeNode<T>::Put(Point const &p, T obj) {
  if (is_leaf_node_) {
    // Put if maximum number of objects in a node don't exceed threshold
    // OR
    // if maximum depth was reached
    // ELSE
    // split node with, according to split parameter

    if (objects_.size() < max_amount_of_objects_in_node_ || (max_depth_ == 0)) {
      objects_.push_back(make_pair(p, obj));

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
    children_[idx]->Put(p, obj);
  }
}

template <typename T>
void KdTreeNode<T>::SplitUsingVaryingMedian() {
  double x_left, y_left, z_left, x_right, y_right, z_right;
  Bound bnd = this->bound_;
  if (!this->is_leaf_node_)
    return;
  Point split_point = GetMedian();

  if (axis_ == 0) {
    x_left = split_point.x_;
    y_left = bnd.near_right_top_point_.y_;
    z_left = bnd.near_right_top_point_.z_;
    x_right = split_point.x_;
    y_right = bnd.far_left_bottom_point_.y_;
    z_right = bnd.far_left_bottom_point_.z_;
  } else if (axis_ == 1) {
    x_left = bnd.near_right_top_point_.x_;
    y_left = split_point.y_;
    z_left = bnd.near_right_top_point_.z_;
    x_right = bnd.far_left_bottom_point_.x_;
    y_right = split_point.y_;
    z_right = bnd.far_left_bottom_point_.z_;
  } else {
    x_left = bnd.near_right_top_point_.x_;
    y_left = bnd.near_right_top_point_.y_;
    z_left = split_point.z_;
    x_right = bnd.far_left_bottom_point_.x_;
    y_right = bnd.far_left_bottom_point_.y_;
    z_right = split_point.z_;
  }

  Point p[2] = {
      Point(bnd.far_left_bottom_point_.x_, bnd.far_left_bottom_point_.y_,
            bnd.far_left_bottom_point_.z_),
      Point(x_right, y_right, z_right)};
  Point e[2] = {
      Point(x_left, y_left, z_left),
      Point(bnd.near_right_top_point_.x_, bnd.near_right_top_point_.y_,
            bnd.near_right_top_point_.z_)};

  for (int i = 0; i < 2; i++) {
    children_[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth_ - 1,
                          max_amount_of_objects_in_node_, split_parameter_);
    children_[i]->axis_ = (axis_ + 1) % 3;
  }

  for (size_t i = 0; i < objects_.size(); i++) {
    int idx = GetChildID(objects_.at(i).first);
    children_[idx]->Put(objects_.at(i).first, objects_.at(i).second);
  }
  objects_.clear();
  this->is_leaf_node_ = false;
}

template <typename T>
void KdTreeNode<T>::SplitUsingSingleXMedian() {
  Bound bnd = this->bound_;
  if (!this->is_leaf_node_)
    return;
  Point split_point = GetMedianOnXAxis();

  Point p[2] = {
      Point(bnd.far_left_bottom_point_.x_, bnd.far_left_bottom_point_.y_,
            bnd.far_left_bottom_point_.z_),
      Point(split_point.x_, bnd.far_left_bottom_point_.y_,
            bnd.far_left_bottom_point_.z_),
  };
  Point e[2] = {
      Point(split_point.x_, bnd.near_right_top_point_.y_,
            bnd.near_right_top_point_.z_),
      Point(bnd.near_right_top_point_.x_, bnd.near_right_top_point_.y_,
            bnd.near_right_top_point_.z_)};

  for (int i = 0; i < 2; i++) {
    children_[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth_ - 1,
                          max_amount_of_objects_in_node_, split_parameter_);
  }

  for (size_t i = 0; i < objects_.size(); i++) {
    int idx = GetChildID(objects_.at(i).first);
    children_[idx]->Put(objects_.at(i).first, objects_.at(i).second);
  }
  objects_.clear();
  this->is_leaf_node_ = false;
}

template <typename T>
void KdTreeNode<T>::SplitUsingSAH() {
  double x_left, y_left, z_left, x_right, y_right, z_right;
  Bound bnd = this->bound_;
  if (!this->is_leaf_node_)
    return;
  Point split_point = GetSAHSplitPoint();

  if (axis_ == 0) {
    x_left = split_point.x_;
    y_left = bnd.near_right_top_point_.y_;
    z_left = bnd.near_right_top_point_.z_;
    x_right = split_point.x_;
    y_right = bnd.far_left_bottom_point_.y_;
    z_right = bnd.far_left_bottom_point_.z_;
  } else if (axis_ == 1) {
    x_left = bnd.near_right_top_point_.x_;
    y_left = split_point.y_;
    z_left = bnd.near_right_top_point_.z_;
    x_right = bnd.far_left_bottom_point_.x_;
    y_right = split_point.y_;
    z_right = bnd.far_left_bottom_point_.z_;
  } else {
    x_left = bnd.near_right_top_point_.x_;
    y_left = bnd.near_right_top_point_.y_;
    z_left = split_point.z_;
    x_right = bnd.far_left_bottom_point_.x_;
    y_right = bnd.far_left_bottom_point_.y_;
    z_right = split_point.z_;
  }
  Point p[2] = {
      Point(bnd.far_left_bottom_point_.x_, bnd.far_left_bottom_point_.y_,
            bnd.far_left_bottom_point_.z_),
      Point(x_right, y_right, z_right)};
  Point e[2] = {
      Point(x_left, y_left, z_left),
      Point(bnd.near_right_top_point_.x_, bnd.near_right_top_point_.y_,
            bnd.near_right_top_point_.z_)};

  for (int i = 0; i < 2; i++) {
    children_[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth_ - 1,
                          max_amount_of_objects_in_node_, split_parameter_);
  }

  for (size_t i = 0; i < objects_.size(); i++) {
    int idx = GetChildID(objects_.at(i).first);
    children_[idx]->Put(objects_.at(i).first, objects_.at(i).second);
  }
  objects_.clear();
  this->is_leaf_node_ = false;
}

template <typename T>
void KdTreeNode<T>::SplitUsingCenterOfSpaceNode() {
  double x_left, y_left, z_left, x_right, y_right, z_right;
  Bound bnd = this->bound_;
  if (!this->is_leaf_node_)
    return;

  if (axis_ == 0) {
    x_left = bnd.Center().x_;
    y_left = bnd.near_right_top_point_.y_;
    z_left = bnd.near_right_top_point_.z_;
    x_right = bnd.Center().x_;
    y_right = bnd.far_left_bottom_point_.y_;
    z_right = bnd.far_left_bottom_point_.z_;
  } else if (axis_ == 1) {
    x_left = bnd.near_right_top_point_.x_;
    y_left = bnd.Center().y_;
    z_left = bnd.near_right_top_point_.z_;
    x_right = bnd.far_left_bottom_point_.x_;
    y_right = bnd.Center().y_;
    z_right = bnd.far_left_bottom_point_.z_;
  } else {
    x_left = bnd.near_right_top_point_.x_;
    y_left = bnd.near_right_top_point_.y_;
    z_left = bnd.Center().z_;
    x_right = bnd.far_left_bottom_point_.x_;
    y_right = bnd.far_left_bottom_point_.y_;
    z_right = bnd.Center().z_;
  }

  Point p[2] = {
      Point(bnd.far_left_bottom_point_.x_, bnd.far_left_bottom_point_.y_,
            bnd.far_left_bottom_point_.z_),
      Point(x_right, y_right, z_right)};
  Point e[2] = {
      Point(x_left, y_left, z_left),
      Point(bnd.near_right_top_point_.x_, bnd.near_right_top_point_.y_,
            bnd.near_right_top_point_.z_)};

  for (int i = 0; i < 2; i++) {
    children_[i] =
        new KdTreeNode<T>(Bound(p[i], e[i]), max_depth_ - 1,
                          max_amount_of_objects_in_node_, split_parameter_);
    children_[i]->axis_ = (axis_ + 1) % 3;
  }

  for (size_t i = 0; i < objects_.size(); i++) {
    int idx = GetChildID(objects_.at(i).first);
    children_[idx]->Put(objects_.at(i).first, objects_.at(i).second);
  }
  objects_.clear();
  this->is_leaf_node_ = false;
}

template <typename T>
SpatialTreeNode<T> **KdTreeNode<T>::GetChildrenNodes() const {
  return (SpatialTreeNode<T> **)children_;
}

template <typename T>
const vector<pair<Point, T>> &KdTreeNode<T>::GetObjects() const {
  return objects_;
}

template <typename T>
size_t KdTreeNode<T>::GetChildrenSize() const {
  if (!IsLeaf())
    return 2u;
  return 0u;
}

template <typename T>
int KdTreeNode<T>::GetChildID(Point p) const {
  if (axis_ == 0) {
    if (p.x_ > this->median_.x_)
      return 1;
    else
      return 0;
  } else if (axis_ == 1) {
    if (p.y_ > this->median_.y_)
      return 1;
    else
      return 0;
  } else {
    if (p.z_ > this->median_.z_)
      return 1;
    else
      return 0;
  }
}

template <typename T>
bool KdTreeNode<T>::IsLeaf() const {
  return is_leaf_node_;
}

template <typename T>
Point KdTreeNode<T>::GetSAHSplitPoint() {
  double objects_count[kNumerOfSpaces], leftside_count[kNumerOfSpaces - 1],
      rightside_count[kNumerOfSpaces - 1];
  double sah, temp;
  Point split_point;
  int axis = 0;

  const Bound &bnd = this->bound_;

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
               node_area_);
  split_point.Set(
      bnd.far_left_bottom_point_.x_ +
          ((bnd.near_right_top_point_.x_ - bnd.far_left_bottom_point_.x_) * 1 /
           kNumerOfSpaces),
      bnd.far_left_bottom_point_.y_, bnd.far_left_bottom_point_.z_);
  for (int i = 1; i < kNumerOfSpaces - 1; i++) {
    temp = kCt +
           kCi * ((AreaOfKthPartOfSpaceNode(i + 1, axis) * leftside_count[0] +
                   AreaOfKthPartOfSpaceNode(kNumerOfSpaces - i - 1, axis)) /
                  node_area_);
    if (temp < sah) {
      sah = temp;
      split_point.Set(
          bnd.far_left_bottom_point_.x_ +
              ((bnd.near_right_top_point_.x_ - bnd.far_left_bottom_point_.x_) *
               (i + 1) / kNumerOfSpaces),
          bnd.far_left_bottom_point_.y_, bnd.far_left_bottom_point_.z_);
      this->axis_ = 0;
    }
  }

  axis = 1;

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
               node_area_);
  split_point.Set(
      bnd.far_left_bottom_point_.x_,
      bnd.far_left_bottom_point_.y_ +
          ((bnd.near_right_top_point_.y_ - bnd.far_left_bottom_point_.y_) * 1 /
           kNumerOfSpaces),
      bnd.far_left_bottom_point_.z_);
  for (int i = 1; i < kNumerOfSpaces - 1; i++) {
    temp = kCt +
           kCi * ((AreaOfKthPartOfSpaceNode(i + 1, axis) * leftside_count[0] +
                   AreaOfKthPartOfSpaceNode(kNumerOfSpaces - i - 1, axis)) /
                  node_area_);
    if (temp < sah) {
      sah = temp;
      split_point.Set(
          bnd.far_left_bottom_point_.x_,
          bnd.far_left_bottom_point_.y_ +
              ((bnd.near_right_top_point_.y_ - bnd.far_left_bottom_point_.y_) *
               (i + 1) / kNumerOfSpaces),
          bnd.far_left_bottom_point_.z_);
      this->axis_ = 1;
    }
  }

  axis = 2;

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
               node_area_);
  split_point.Set(
      bnd.far_left_bottom_point_.x_, bnd.far_left_bottom_point_.y_,
      bnd.far_left_bottom_point_.z_ +
          ((bnd.near_right_top_point_.z_ - bnd.far_left_bottom_point_.z_) * 1 /
           kNumerOfSpaces));
  for (int i = 1; i < kNumerOfSpaces - 1; i++) {
    temp = kCt +
           kCi * ((AreaOfKthPartOfSpaceNode(i + 1, axis) * leftside_count[0] +
                   AreaOfKthPartOfSpaceNode(kNumerOfSpaces - i - 1, axis)) /
                  node_area_);
    if (temp < sah) {
      sah = temp;
      split_point.Set(
          bnd.far_left_bottom_point_.x_, bnd.far_left_bottom_point_.y_,
          bnd.far_left_bottom_point_.z_ +
              ((bnd.near_right_top_point_.z_ - bnd.far_left_bottom_point_.z_) *
               (i + 1) / kNumerOfSpaces));
      this->axis_ = 2;
    }
  }

  return split_point;
}

template <typename T>
Point KdTreeNode<T>::GetMedian() {
  auto &sorted = objects_;
  if (axis_ == 0) {
    std::nth_element(sorted.begin(), sorted.begin() + sorted.size() / 2,
                     sorted.end(), PointCompareX);

  } else if (axis_ == 1) {
    std::nth_element(sorted.begin(), sorted.begin() + sorted.size() / 2,
                     sorted.end(), PointCompareY);
  } else {
    std::nth_element(sorted.begin(), sorted.begin() + sorted.size() / 2,
                     sorted.end(), PointCompareZ);
  }
  this->median_ = sorted.at(sorted.size() / 2).first;
  return sorted.at(sorted.size() / 2).first;
}

template <typename T>
Point KdTreeNode<T>::GetMedianOnXAxis() {
  auto &sorted = objects_;

  std::nth_element(sorted.begin(), sorted.begin() + sorted.size() / 2,
                   sorted.end(), PointCompareX);
  this->median_ = sorted.at(sorted.size() / 2).first;
  return sorted.at(sorted.size() / 2).first;
}

template <typename T>
double KdTreeNode<T>::AreaOfKthPartOfSpaceNode(int k, int axis) {
  if (axis == 0) {
    return this->node_area_ -
           k / kNumerOfSpaces *
               (this->bound_.Height() * this->bound_.Length()) -
           k / kNumerOfSpaces * (this->bound_.Width() * this->bound_.Length());
  } else if (axis == 1) {
    return this->node_area_ -
           k / kNumerOfSpaces * (this->bound_.Height() * this->bound_.Width()) -
           k / kNumerOfSpaces * (this->bound_.Width() * this->bound_.Length());
  } else if (axis == 2) {
    return this->node_area_ - (k / kNumerOfSpaces * (this->bound_.Height() *
                                                     this->bound_.Width()) +
                               k / kNumerOfSpaces * (this->bound_.Height() *
                                                     this->bound_.Length()));
  } else {
    return -1;
  }
}
}  // namespace spatial_organization
}  // namespace bdm
#endif  // SPATIAL_ORGANIZATION_KD_TREE_NODE_H_
