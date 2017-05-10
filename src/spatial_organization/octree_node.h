#ifndef SPATIAL_ORGANIZATION_OCTREE_NODE_H_
#define SPATIAL_ORGANIZATION_OCTREE_NODE_H_

#include <utility>
#include <vector>
#include "spatial_organization/spatial_tree_node.h"

namespace bdm {
namespace spatial_organization {

using std::vector;
using std::make_pair;

/// Spatial tree descendant.
/// Implements octree. It organizes number of points in space with 8 dimensions.
/// @tparam T - type of the object to be stored in the tree
template <typename T>
class OctreeNode : public SpatialTreeNode<T> {
 public:
  /// Empty constructor, initializes with bounds (0, 0, 0, 1, 1, 1),
  /// maximum depth 10,
  /// maximum amount of objects within 1 node 1000
  /// @tparam T - type of the object to be stored in the tree
  OctreeNode();

  /// Constructor
  /// @tparam T  - type of the object to be stored in the tree
  /// @param bnd - Bound of the node
  /// @param max_depth - maximum possible depth of the tree. After reaching that
  /// point, nodes won't split
  /// @param max_amount_of_objects - maximum number of object which can be
  /// stored
  /// in 1 node.
  /// In our case, amount of object acts as splitting criteria
  OctreeNode(const Bound &bnd, int max_depth, int max_amount_of_objects);

  /// Constructor
  /// @tparam T  - type of the object to be stored in the tree
  /// @param max_depth - maximum possible depth of the tree. After reaching that
  /// point, nodes won't split
  /// @param max_amount_of_objects - maximum number of object which can be
  /// stored
  /// in 1 node.
  /// In our case, amount of object acts as splitting criteria
  OctreeNode(int max_depth, int max_amount_of_objects);

  /// destructor
  /// @tparam T - type of the object to be stored in the tree
  ~OctreeNode();

  bool IsLeaf() const override;

  /// Adds new object to the tree
  /// @param p - position of the new object
  /// @param obj - object itself
  void Put(Point const &p, T obj) override;

  T At(Point const &p) const;

  size_t Size() const;

 private:
  bool is_leaf_node_;
  OctreeNode<T> *children_[8];
  vector<pair<Point, T> > objects_;
  size_t max_depth_;
  size_t max_amount_of_objects_in_node_;

  /// Splits node to 8 equal subspaces
  /// @tparam T - type of objects
  void Split();

  /// returns in what octa subspace point p is located
  /// @tparam T - type of the object
  /// @param p - point in space
  /// @return number of subspace
  int GetChildID(Point const &p) const;

  SpatialTreeNode<T> **GetChildrenNodes() const override;

  const vector<pair<Point, T> > &GetObjects() const override;

  size_t GetChildrenSize() const override;
};

template <typename T>
OctreeNode<T>::OctreeNode() {
  OctreeNode<T>(Bound(0, 0, 0, 1, 1, 1), 10, 1000);
}

template <typename T>
OctreeNode<T>::OctreeNode(int max_depth, int max_amount_of_objects)
    : SpatialTreeNode<T>(Bound(-Param::kInfinity, -Param::kInfinity,
                               -Param::kInfinity, Param::kInfinity,
                               Param::kInfinity, Param::kInfinity)),
      is_leaf_node_(true),
      max_depth_(max_depth),
      max_amount_of_objects_in_node_(max_amount_of_objects) {}

template <typename T>
OctreeNode<T>::OctreeNode(const Bound &bnd, int max_depth,
                          int max_amount_of_objects)
    : SpatialTreeNode<T>(bnd),
      is_leaf_node_(true),
      max_depth_(max_depth),
      max_amount_of_objects_in_node_(max_amount_of_objects) {}

template <typename T>
OctreeNode<T>::~OctreeNode() {
  if (!IsLeaf()) {
    for (int i = 0; i < 8; i++)
      if (children_[i] != nullptr) {
        delete children_[i];
        children_[i] = nullptr;
      }
  }
}

template <typename T>
bool OctreeNode<T>::IsLeaf() const {
  return is_leaf_node_;
}

template <typename T>
void OctreeNode<T>::Put(Point const &p, T obj) {
  if (is_leaf_node_) {
    // Insert
    if (objects_.size() < max_amount_of_objects_in_node_ || (max_depth_ == 0)) {
      objects_.push_back(make_pair(p, obj));
    } else {
      // SplitUsingVaryingMedian
      Split();
      Put(p, obj);
    }
  } else {
    int idx = GetChildID(p);
    children_[idx]->Put(p, obj);
  }
}

template <typename T>
void OctreeNode<T>::Split() {
  if (!this->is_leaf_node_)
    return;
  Point center = this->bound_.Center();
  const Bound &bnd = this->bound_;
  Point p[8] = {Point(center.x_, center.y_, center.z_),
                Point(center.x_, bnd.Left(), center.z_),
                Point(center.x_, bnd.Left(), bnd.Bottom()),
                Point(center.x_, center.y_, bnd.Bottom()),
                Point(bnd.Far(), center.y_, center.z_),
                Point(bnd.Far(), bnd.Left(), center.z_),
                Point(bnd.Far(), bnd.Left(), bnd.Bottom()),
                Point(bnd.Far(), center.y_, bnd.Bottom())};
  Point e[8] = {Point(bnd.Near(), bnd.Right(), bnd.Top()),
                Point(bnd.Near(), center.y_, bnd.Top()),
                Point(bnd.Near(), center.y_, center.z_),
                Point(bnd.Near(), bnd.Right(), center.z_),
                Point(center.x_, bnd.Right(), bnd.Top()),
                Point(center.x_, center.y_, bnd.Top()),
                Point(center.x_, center.y_, center.z_),
                Point(center.x_, bnd.Right(), center.z_)};

  for (int i = 0; i < 8; i++) {
    children_[i] = new OctreeNode<T>(Bound(p[i], e[i]), max_depth_ - 1,
                                     max_amount_of_objects_in_node_);
  }

  for (size_t i = 0; i < objects_.size(); i++) {
    int idx = GetChildID(objects_.at(i).first);
    children_[idx]->Put(objects_.at(i).first, objects_.at(i).second);
  }
  this->is_leaf_node_ = false;
}

template <typename T>
int OctreeNode<T>::GetChildID(Point const &p) const {
  Point center = this->bound_.Center();
  int result = 0;
  if (p.y_ >= center.y_) {
    if (p.z_ >= center.z_) {
      result = 0;
    } else {
      result = 3;
    }
  } else {
    if (p.z_ >= center.z_) {
      result = 1;
    } else {
      result = 2;
    }
  }
  if (p.x_ < center.x_) {
    result += 4;
  }
  return result;
}

template <typename T>
T OctreeNode<T>::At(Point const &p) const {
  if (this->is_leaf_node_) {
    for (int i = 0; i < objects_.size(); i++)
      if (p.equals(objects_.at(i).first))
        return objects_.at(i).second;
  } else {
    int idx = GetChildID(p);
    return children_[idx]->At(p);
  }
  return NULL;
}

template <typename T>
size_t OctreeNode<T>::Size() const {
  if (this->is_leaf_node_) {
    return objects_.size();
  } else {
    size_t sum = 0;
    for (int i = 0; i < 8; i++) {
      sum += children_[i]->Size();
    }
    return sum;
  }
}

template <typename T>
SpatialTreeNode<T> **OctreeNode<T>::GetChildrenNodes() const {
  return (SpatialTreeNode<T> **)children_;
}

template <typename T>
const vector<pair<Point, T> > &OctreeNode<T>::GetObjects() const {
  return objects_;
}

template <typename T>
size_t OctreeNode<T>::GetChildrenSize() const {
  return !IsLeaf() ? 8u : 0u;
}
}  //  namespace spatial_organization
}  //  namespace bdm
#endif  // SPATIAL_ORGANIZATION_OCTREE_NODE_H_
