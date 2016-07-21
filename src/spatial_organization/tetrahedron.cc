#include "spatial_organization/tetrahedron.h"

#include <algorithm>

#include "matrix.h"
#include "string_util.h"
#include "spatial_organization/exact_vector.h"
#include "spatial_organization/rational.h"
#include "spatial_organization/triangle_3d.h"
#include "spatial_organization/open_triangle_organizer.h"
#include "spatial_organization/flat_tetrahedron.h"

namespace bdm {
namespace spatial_organization {

template<class T>
std::shared_ptr<Tetrahedron<T>> Tetrahedron<T>::create(
    const std::shared_ptr<Triangle3D<T>>& one_triangle,
    SpaceNode<T>* fourth_point,
    const std::shared_ptr<OpenTriangleOrganizer<T>>& oto) {
  Tetrahedron<T>* raw_pointer = new Tetrahedron();
  std::shared_ptr<Tetrahedron<T>> tetrahedron(raw_pointer);
  tetrahedron->initializationHelper(one_triangle, fourth_point, oto);
  return tetrahedron;
}

template<class T>
std::shared_ptr<Tetrahedron<T>> Tetrahedron<T>::create(const std::shared_ptr<Triangle3D<T>>& triangle_a,
                                              const std::shared_ptr<Triangle3D<T>>& triangle_b,
                                              const std::shared_ptr<Triangle3D<T>>& triangle_c,
                                              const std::shared_ptr<Triangle3D<T>>& triangle_d,
                                              SpaceNode<T>* node_a,
                                              SpaceNode<T>* node_b,
                                              SpaceNode<T>* node_c,
                                              SpaceNode<T>* node_d) {
  Tetrahedron<T>* raw_pointer = new Tetrahedron();
  std::shared_ptr<Tetrahedron<T>> tetrahedron(raw_pointer);
  tetrahedron->initializationHelper(triangle_a, triangle_b, triangle_c, triangle_d, node_a,
                                    node_b, node_c, node_d);
  return tetrahedron;
}

template<class T>
std::shared_ptr<Tetrahedron<T> > Tetrahedron<T>::createInitialTetrahedron(
    SpaceNode<T>* a, SpaceNode<T>* b,
    SpaceNode<T>* c, SpaceNode<T>* d,
    const std::shared_ptr<OpenTriangleOrganizer<T>>& simple_oto) {

  std::shared_ptr<Tetrahedron<T>> null_tetrahedron(nullptr);
  auto triangle_a = Triangle3D<T>::create(b, c, d, null_tetrahedron, null_tetrahedron);
  auto triangle_b = Triangle3D<T>::create(a, c, d, null_tetrahedron, null_tetrahedron);
  auto triangle_c = Triangle3D<T>::create(a, b, d, null_tetrahedron, null_tetrahedron);
  auto triangle_d = Triangle3D<T>::create(a, b, c, null_tetrahedron, null_tetrahedron);
  auto ret = Tetrahedron<T>::create(triangle_a, triangle_b, triangle_c, triangle_d, a, b, c, d);

  SpaceNode<T>* null_spacenode(nullptr);
  Tetrahedron<T>::create(triangle_a, null_spacenode, simple_oto);
  Tetrahedron<T>::create(triangle_b, null_spacenode, simple_oto);
  Tetrahedron<T>::create(triangle_c, null_spacenode, simple_oto);
  Tetrahedron<T>::create(triangle_d, null_spacenode, simple_oto);
  return ret;
}

template<class T>
Tetrahedron<T>::~Tetrahedron(){
}

template<class T>
void Tetrahedron<T>::calculateCircumSphere() {
  if (!isInfinite()) {
    circum_center_ = {0.0, 0.0, 0.0};
    circum_center_is_null_ = true;
    computeCircumsphereCenterAndVolume();
    computeRadius();
  }
}

template<class T>
void Tetrahedron<T>::updateCirumSphereAfterNodeMovement(
    SpaceNode<T>* moved_node) {
  size_t node_number = getNodeNumber(moved_node);
  if (!isInfinite()) {
    circum_center_ = {0.0, 0.0, 0.0};
    circum_center_is_null_ = true;
    computeCircumsphereCenterAndVolume();
    computeRadius();
  }
  for (size_t i = 0; i < adjacent_triangles_.size(); i++) {
    if (i != node_number)
      adjacent_triangles_[i]->informAboutNodeMovement();
  }
}

template<class T>
int Tetrahedron<T>::orientation(const std::array<double, 3>& point) {
  if (!isInfinite()) {
    auto dummy = Matrix::subtract(circum_center_, point);
    double dum = Matrix::dot(dummy, dummy);
    if (dum > squared_radius_ + tolerance_) {
      return -1;
    } else if (dum < squared_radius_ - tolerance_) {
      return 1;
    } else {
      int result = orientationExact(point);
      if ((result != 0) && ((result == 1) ^ (dum < squared_radius_))) {
        double difference = std::abs(squared_radius_ - dum);
        difference /= tolerance_;
        orientationExact(point);
        calculateCircumSphere();
      }
      return result;
    }
  } else {
    auto inner_tetrahedron = getAdjacentTetrahedron(0);
    adjacent_triangles_[0]->updatePlaneEquationIfNecessary();
    int orientation;
    if (inner_tetrahedron != nullptr) {
      if (inner_tetrahedron->isInfinite())
        return 1;
      auto position = inner_tetrahedron->getOppositeNode(adjacent_triangles_[0])->getPosition();
      orientation = adjacent_triangles_[0]->orientation(point, position);
    } else {
      orientation = adjacent_triangles_[0]->orientationToUpperSide(point);
    }
    if (orientation == 0) {
      return adjacent_triangles_[0]->circleOrientation(point);
    } else {
      return -orientation;
    }
  }
}

template<class T>
bool Tetrahedron<T>::isTrulyInsideSphere(const std::array<double, 3>& point) {
  return orientation(point) > 0;
}

template<class T>
bool Tetrahedron<T>::isInsideSphere(const std::array<double, 3>& point) {
  return orientation(point) >= 0;
}

template<class T>
std::string Tetrahedron<T>::toString() const {
  std::ostringstream str_stream;
  str_stream << "(";
//  for (size_t i = 0; i < adjacent_nodes_.size(); i++) {
//    str_stream << StringUtil::toStr(adjacent_nodes_[i]) << ", ";
//  }
//  str_stream << StringUtil::toStr(adjacent_nodes_);
//  str_stream << ", ";
  str_stream << StringUtil::toStr(circum_center_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(squared_radius_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(tolerance_);
  str_stream << ")";
  return str_stream.str();
}

template<class T>
bool Tetrahedron<T>::equalTo(const std::shared_ptr<Tetrahedron<T>>& other) {
  return other.get() == this;
}

template<class T>
std::array<std::shared_ptr<Triangle3D<T> >, 4> Tetrahedron<T>::getAdjacentTriangles() const {
  return adjacent_triangles_;
}

template<class T>
bool Tetrahedron<T>::isAdjacentTo(SpaceNode<T>* node) const {
  return (adjacent_nodes_[0] == node) || (adjacent_nodes_[1] == node)
      || (adjacent_nodes_[2] == node) || (adjacent_nodes_[3] == node);
}

template<class T>
std::shared_ptr<Tetrahedron<T> > Tetrahedron<T>::walkToPoint(
    const std::array<double, 3>& coordinate, const std::array<int, 4>& triangle_order) {
  if (!isInfinite()) {
    for (size_t i = 0; i < triangle_order.size(); i++) {
      int pos = triangle_order[i];
      auto current_triangle = adjacent_triangles_[pos];
      current_triangle->updatePlaneEquationIfNecessary();
      int orientation = current_triangle->orientation(adjacent_nodes_[pos]->getPosition(),
                                                      coordinate);
      if (orientation < 0) {
        return current_triangle->getOppositeTetrahedron(this->shared_from_this());
      } else if (orientation == 0) {
        auto opposite_tetrahedron = current_triangle->getOppositeTetrahedron(
            this->shared_from_this());
        if (opposite_tetrahedron->isInfinite() && isTrulyInsideSphere(coordinate)) {
          testPosition(coordinate);
          return opposite_tetrahedron;
        }
      }
    }
  } else {
    if (!isInsideSphere(coordinate)) {
      return adjacent_triangles_[0]->getOppositeTetrahedron(this->shared_from_this());
    }
  }
  testPosition(coordinate);
  return this->shared_from_this();
}

template<class T>
int Tetrahedron<T>::getEdgeNumber(int node_number_1, int node_number_2) {
  int subtract = ((node_number_1 == 0) ? 1 : ((node_number_2 == 0) ? 1 : 0));
  return node_number_1 + node_number_2 - subtract;
}

template<class T>
std::vector<std::shared_ptr<Tetrahedron<T> > > Tetrahedron<T>::remove2FlatTetrahedra(
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron_a,
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron_b) {
  auto triangle_list_a = tetrahedron_a->getAdjacentTriangles();
  auto triangle_list_b = tetrahedron_b->getAdjacentTriangles();
  std::vector<std::shared_ptr<Tetrahedron<T>> > adjacent_tetrahedra;
  std::array<double, 3> outer_triangles_a = { -1.0, -1.0, -1.0 };
  std::array<double, 3> outer_triangles_b = { -1.0, -1.0, -1.0 };
  int outer_triangle_count = 0;
  for (size_t i = 0; i < 4; i++) {
    bool possible = true;
    for (size_t j = 0; (j < 4) && (possible); j++) {
      possible = triangle_list_a[i] != triangle_list_b[j];
    }
    if (possible) {
      outer_triangles_a[outer_triangle_count] = i;
      for (size_t j = 0; j < 4; j++) {
        if (triangle_list_a[i]->isSimilarTo(triangle_list_b[j]))
          outer_triangles_b[outer_triangle_count] = j;
      }
      outer_triangle_count++;
    }
  }
  tetrahedron_a->remove();
  tetrahedron_b->remove();
  for (int i = 0; i < outer_triangle_count; i++) {
    std::shared_ptr<Tetrahedron<T>> null_tetrahedron(nullptr);
    auto a = triangle_list_a[outer_triangles_a[i]]->getOppositeTetrahedron(null_tetrahedron);
    // if list does not contain element
    if (std::find(adjacent_tetrahedra.begin(), adjacent_tetrahedra.end(), a)
        == adjacent_tetrahedra.end()) {
      adjacent_tetrahedra.push_back(a);
    }

    auto b = triangle_list_b[outer_triangles_b[i]]->getOppositeTetrahedron(null_tetrahedron);
    if (std::find(adjacent_tetrahedra.begin(), adjacent_tetrahedra.end(), b)
        == adjacent_tetrahedra.end()) {
      adjacent_tetrahedra.push_back(b);
    }
    a->replaceTriangle(triangle_list_a[outer_triangles_a[i]],
                       triangle_list_b[outer_triangles_b[i]]);
  }
  return adjacent_tetrahedra;
}

template<class T>
std::array<std::shared_ptr<Tetrahedron<T> >, 3> Tetrahedron<T>::flip2to3(
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron_a,
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron_b) {
  int connecting_triangle_number = tetrahedron_a->getConnectingTriangleNumber(tetrahedron_b);
  auto connecting_triangle = tetrahedron_a->getAdjacentTriangles()[connecting_triangle_number];
  auto lower_node = tetrahedron_b->getOppositeNode(connecting_triangle);
  int convex_position = 1;
  if (lower_node != nullptr) {
    convex_position = tetrahedron_a->isInConvexPosition(lower_node->getPosition(),
                                                        connecting_triangle_number);
  }
  if (convex_position >= 0) {
    bool check_for_flat_tetrahedra = convex_position == 0;
    auto upper_triangles = tetrahedron_a->getTouchingTriangles(connecting_triangle);
    auto lower_triangles = tetrahedron_b->getTouchingTriangles(connecting_triangle);
    auto upper_node = tetrahedron_a->getAdjacentNodes()[connecting_triangle_number];
    std::array<std::shared_ptr<Triangle3D<T>>, 3> new_triangles;
    auto connecting_triangle_nodes = connecting_triangle->getNodes();
    std::shared_ptr<Tetrahedron<T>> null_tetrahedron;
    for (size_t i = 0; i < 3; i++) {
      new_triangles[i] = Triangle3D<T>::create(upper_node, lower_node, connecting_triangle_nodes[i],
                                               null_tetrahedron, null_tetrahedron);
    }
    tetrahedron_a->remove();
    tetrahedron_b->remove();
    std::array<std::shared_ptr<Tetrahedron<T>>, 3> ret;
    for (size_t i = 0; i < 3; i++) {
      // make sure a node at position 0 is always inserted at position
      // 0, if it is part of the connecting triangle:
      int a = (i + 1) % 3;
      int b = (i + 2) % 3;
      if (b == 0) {
        b = 2;
        a = 0;
      }
      auto position = lower_node->getPosition();
      if (check_for_flat_tetrahedra && upper_triangles[i]->orientation(position, position) == 0) {
        ret[i] = FlatTetrahedron<T>::create(new_triangles[b], upper_triangles[i],
                                            lower_triangles[i], new_triangles[a],
                                            connecting_triangle_nodes[a], lower_node, upper_node,
                                            connecting_triangle_nodes[b]);
      } else {
        ret[i] = Tetrahedron<T>::create(new_triangles[b], upper_triangles[i], lower_triangles[i],
                                        new_triangles[a], connecting_triangle_nodes[a], lower_node,
                                        upper_node, connecting_triangle_nodes[b]);
      }
    }
    return ret;
  }
  // todo find better way to return null array
  std::array<std::shared_ptr<Tetrahedron<T>>, 3> ret;
  std::shared_ptr<Tetrahedron<T>> null_element(nullptr);
  for (size_t i = 0; i < ret.size(); i++) {
    ret[i] = null_element;
  }
  return ret;
}

template<class T>
std::array<std::shared_ptr<Tetrahedron<T> >, 2> Tetrahedron<T>::flip3to2(
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron_a,
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron_b,
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron_c) {
  std::array<SpaceNode<T>*, 3> newTriangleNodes;

  int num_a = tetrahedron_a->getConnectingTriangleNumber(tetrahedron_b);
  int num_b = tetrahedron_b->getConnectingTriangleNumber(tetrahedron_c);
  int num_c = tetrahedron_c->getConnectingTriangleNumber(tetrahedron_a);

  newTriangleNodes[0] = tetrahedron_a->getAdjacentNodes()[num_a];
  newTriangleNodes[1] = tetrahedron_b->getAdjacentNodes()[num_b];
  newTriangleNodes[2] = tetrahedron_c->getAdjacentNodes()[num_c];

  auto upperNode = tetrahedron_a->getFirstOtherNode(newTriangleNodes[0], newTriangleNodes[1]);
  auto lowerNode = tetrahedron_a->getSecondOtherNode(newTriangleNodes[0], newTriangleNodes[1]);

  std::shared_ptr<Tetrahedron<T>> null_tetrahdron(nullptr);
  auto newTriangle = Triangle3D<T>::create(newTriangleNodes[0], newTriangleNodes[1],
                                           newTriangleNodes[2], null_tetrahdron, null_tetrahdron);

  std::array<std::shared_ptr<Tetrahedron<T>>, 2> ret;

  auto tetraAOppTriangleLow = tetrahedron_a->getOppositeTriangle(lowerNode);
  auto tetraBOppTriangleLow = tetrahedron_b->getOppositeTriangle(lowerNode);
  auto tetraCOppTriangleLow = tetrahedron_c->getOppositeTriangle(lowerNode);

  auto tetraAOppTriangleUp = tetrahedron_a->getOppositeTriangle(upperNode);
  auto tetraBOppTriangleUp = tetrahedron_b->getOppositeTriangle(upperNode);
  auto tetraCOppTriangleUp = tetrahedron_c->getOppositeTriangle(upperNode);

  bool flat = tetrahedron_a->isFlat() && tetrahedron_b->isFlat() && tetrahedron_c->isFlat();
  tetrahedron_a->remove();
  tetrahedron_b->remove();
  tetrahedron_c->remove();

  if (!flat) {
    ret[0] = Tetrahedron<T>::create(newTriangle, tetraAOppTriangleLow, tetraBOppTriangleLow,
                                    tetraCOppTriangleLow, upperNode, newTriangleNodes[2],
                                    newTriangleNodes[0], newTriangleNodes[1]);
    ret[1] = Tetrahedron<T>::create(newTriangle, tetraAOppTriangleUp, tetraBOppTriangleUp,
                                    tetraCOppTriangleUp, lowerNode, newTriangleNodes[2],
                                    newTriangleNodes[0], newTriangleNodes[1]);
  } else {
    ret[0] = FlatTetrahedron<T>::create(newTriangle, tetraAOppTriangleLow, tetraBOppTriangleLow,
                                        tetraCOppTriangleLow, upperNode, newTriangleNodes[2],
                                        newTriangleNodes[0], newTriangleNodes[1]);
    ret[1] = FlatTetrahedron<T>::create(newTriangle, tetraAOppTriangleUp, tetraBOppTriangleUp,
                                        tetraCOppTriangleUp, lowerNode, newTriangleNodes[2],
                                        newTriangleNodes[0], newTriangleNodes[1]);
  }
  return ret;
}

template<class T>
std::array<T*, 4> Tetrahedron<T>::getVerticeContents() const {
  std::array<T*, 4> ret;
  for (size_t i = 0; i < 4; i++) {
    if (adjacent_nodes_[i] != nullptr) {
      ret[i] = adjacent_nodes_[i]->getUserObject();
    } else {
      ret[i] = nullptr; // todo - should be return null!
    }
  }
  return ret;
}

template<class T>
bool Tetrahedron<T>::isInfinite() const {
  return adjacent_nodes_[0] == nullptr;
}

template<class T>
void Tetrahedron<T>::changeCrossSection(int number, double new_value) {
  double change = new_value - cross_section_areas_[number];
  if (change != 0) {
    adjacent_edges_[number]->changeCrossSectionArea(change);
  }
  cross_section_areas_[number] = new_value;
}

template<class T>
void Tetrahedron<T>::updateCrossSectionAreas() {
  if (isInfinite()) {
    for (int i = 0; i < 6; i++) {
      changeCrossSection(i, 0.0);
    }
  } else {
    std::array<std::array<double, 3>, 6> line_middles;
    std::array<std::array<double, 3>, 6> line_vectors;
    std::array<std::array<double, 3>, 4> area_middles;
    std::array<double, 3> tetra_middle = { 0.0, 0.0, 0.0 };
    std::array<std::array<double, 3>, 4> positions;

    for (size_t i = 0; i < 4; i++) {
      positions[i] = adjacent_nodes_[i]->getPosition();
    }
    // i: dimension: x, y, z
    for (int i = 0; i < 3; i++) {
      for (int j = 0, line_counter = 0; j < 4; j++) {
        tetra_middle[i] += positions[j][i];
        for (int k = j + 1; k < 4; k++, line_counter++) {
          line_middles[line_counter][i] = (positions[j][i] + positions[k][i]) * 0.5;
          line_vectors[line_counter][i] = (positions[j][i] - positions[k][i]);
        }
        area_middles[j][i] = 0.0;
        for (int k = 0; k < 4; k++) {
          if (k != j) {
            area_middles[j][i] += positions[k][i];
          }
        }
        area_middles[j][i] /= 3;
      }
      tetra_middle[i] *= 0.25;
    }

    // now compute the areas for each pair of nodes:
    for (int j = 0, counter = 5; j < 4; j++) {
      for (int k = j + 1; k < 4; k++, counter--) {
        auto diff_1 = Matrix::subtract(line_middles[counter], tetra_middle);
        auto diff_2 = Matrix::subtract(area_middles[j], area_middles[k]);
        auto cross_product = Matrix::crossProduct(diff_1, diff_2);
        auto dot_product = Matrix::dot(cross_product, line_vectors[counter]);
        auto new_cross_section = std::abs(dot_product / Matrix::norm(line_vectors[counter]));

        changeCrossSection(counter, new_cross_section);
      }
    }
  }
}

template<class T>
void Tetrahedron<T>::calculateVolume() {
  changeVolume(std::abs(Matrix::det(getPlaneNormals()) / 6.0));
}

template<class T>
int Tetrahedron<T>::orientationExact(const std::array<double, 3>& position) const {
  if (isInfinite()) {
    return 1;
  } else {
    std::array<std::shared_ptr<ExactVector>, 4> points;
    for (size_t i = 0; i < points.size(); i++) {
      points[i] = ExactVector::create(adjacent_nodes_[i]->getPosition());
    }
    std::array<std::shared_ptr<ExactVector>, 3> normals;
    for (size_t j = 0; j < normals.size(); j++) {
      normals[j] = points[j + 1]->subtract(points[0]);
    }

    auto det = ExactVector::det(normals);

    auto half = Rational::create(1, 2);
    std::array<std::shared_ptr<Rational>, 3> offsets;
    for (size_t j = 0; j < offsets.size(); j++) {
      offsets[j] = points[0]->add(points[j + 1])->dotProduct(normals[j])->multiplyBy(half);
    }
    auto circum_center = Triangle3D<T>::calculate3PlaneXPoint(normals, offsets, det);
    auto dummy = circum_center->subtract(points[0]);
    auto squared_radius = dummy->dotProduct(dummy);
    dummy = circum_center->subtract(ExactVector::create(position));
    auto distance = dummy->dotProduct(dummy);
    return squared_radius->compareTo(distance);
  }
}

template<class T>
void Tetrahedron<T>::replaceTriangle(const std::shared_ptr<Triangle3D<T> >& old_triangle,
                                     const std::shared_ptr<Triangle3D<T> >& new_triangle) {
  new_triangle->addTetrahedron(this->shared_from_this());
  auto other_tetrahedron = new_triangle->getOppositeTetrahedron(this->shared_from_this());
  size_t triangle_number = getTriangleNumber(old_triangle);
  for (size_t i = 0, position = (triangle_number + 2) % 4, last_position = (triangle_number + 1)
      % 4; i < 3; i++) {
    int edge_number = getEdgeNumber(last_position, position);
    auto other_edge = other_tetrahedron->getEdge(adjacent_nodes_[last_position],
                                                 adjacent_nodes_[position]);
    if (other_edge != adjacent_edges_[edge_number]) {
      adjacent_edges_[edge_number]->removeTetrahedron(this->shared_from_this());
      other_edge->addTetrahedron(this->shared_from_this());
      adjacent_edges_[edge_number] = other_edge;
    }

    last_position = position;
    position = (position + 1) % 4;
    if (position == triangle_number) {
      position = (position + 1) % 4;
    }
  }
  adjacent_triangles_[triangle_number] = new_triangle;
  new_triangle->wasCheckedAlready(-1);
}

template<class T>
int Tetrahedron<T>::getNodeNumber(SpaceNode<T>* node) const {
  for (size_t i = 0; i < adjacent_nodes_.size(); i++) {
    if (adjacent_nodes_[i] == node) {
      return i;
    }
  }
  throw std::invalid_argument(
      "The node " + node->toString() + " is not adjacent to " + toString() + "!");
}

template<class T>
int Tetrahedron<T>::getTriangleNumber(const std::shared_ptr<Triangle3D<T> >& triangle) const {
  for (size_t i = 0; i < adjacent_triangles_.size(); i++) {
    if (adjacent_triangles_[i] == triangle) {
      return i;
    }
  }
  throw std::runtime_error(
      "The triangle " + triangle->toString() + " is not adjacent to " + toString() + "!");
}

template<class T>
Edge<T>* Tetrahedron<T>::getEdge(int node_number_1, int node_number_2) const {
  return adjacent_edges_[getEdgeNumber(node_number_1, node_number_2)];
}

template<class T>
Edge<T>* Tetrahedron<T>::getEdge(SpaceNode<T>* a,
                                                  SpaceNode<T>* b) const {
  return adjacent_edges_[getEdgeNumber(a, b)];
}

template<class T>
int Tetrahedron<T>::getEdgeNumber(SpaceNode<T>* a,
                                  SpaceNode<T>* b) const {
  return getEdgeNumber(getNodeNumber(a), getNodeNumber(b));
}

template<class T>
std::shared_ptr<Triangle3D<T> > Tetrahedron<T>::getOppositeTriangle(
    SpaceNode<T>* node) const {
  for (size_t i = 0; i < 4; i++) {
    if (adjacent_nodes_[i] == node)
      return adjacent_triangles_[i];
  }
  throw std::runtime_error(
      "The SpaceNode " + node->toString() + " is not adjacent to the Tetrahedron " + toString());
}

template<class T>
SpaceNode<T>* Tetrahedron<T>::getOppositeNode(
    const std::shared_ptr<Triangle3D<T>>& triangle) const {
  for (size_t i = 0; i < 4; i++) {
    if (adjacent_triangles_[i] == triangle) {
      return adjacent_nodes_[i];
    }
  }
  throw std::runtime_error(
      "The Triangle " + triangle->toString() + " is not adjacent to the Tetrahedron " + toString());
}

template<class T>
std::shared_ptr<Triangle3D<T> > Tetrahedron<T>::getConnectingTriangle(
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron) const {
  for (size_t i = 0; i < 4; i++) {
    if (adjacent_triangles_[i]->isAdjacentTo(tetrahedron))
      return adjacent_triangles_[i];
  }
  throw std::runtime_error(
      "The Tetrahedron " + tetrahedron->toString() + " is not adjacent to " + toString() + "!");
}

template<class T>
int Tetrahedron<T>::getConnectingTriangleNumber(
    const std::shared_ptr<Tetrahedron<T> >& tetrahedron) const {
  for (size_t i = 0; i < 4; i++) {
    if (adjacent_triangles_[i]->isAdjacentTo(tetrahedron))
      return i;
  }
  throw std::runtime_error(
      "The Tetrahedron " + tetrahedron->toString() + " is not adjacent to " + toString() + "!");
}

template<class T>
std::array<std::shared_ptr<Triangle3D<T> >, 3> Tetrahedron<T>::getTouchingTriangles(
    const std::shared_ptr<Triangle3D<T> >& base) const {
  std::array<std::shared_ptr<Triangle3D<T> >, 3> ret;
  auto triangle_nodes = base->getNodes();
  for (size_t i = 0; i < 3; i++) {
    ret[i] = getOppositeTriangle(triangle_nodes[i]);
  }
  return ret;
}

template<class T>
void Tetrahedron<T>::remove() {
  valid_ = false;
  for (size_t i = 0; i < 4; i++) {
    if (adjacent_nodes_[i] != nullptr) {
      adjacent_nodes_[i]->changeVolume(-volume_ / 4.0);
      adjacent_nodes_[i]->removeTetrahedron(this->shared_from_this());
    }
    auto opposite = getAdjacentTetrahedron(i);
    if (opposite != nullptr && !isInfinite() && opposite->isInfinite()) {
      adjacent_triangles_[i]->orientToSide(adjacent_nodes_[i]->getPosition());
    }
    adjacent_triangles_[i]->removeTetrahedron(this->shared_from_this());
  }
  for (size_t i = 0; i < 6; i++) {
    if (adjacent_edges_[i] != nullptr) {
      adjacent_edges_[i]->changeCrossSectionArea(-cross_section_areas_[i]);
      adjacent_edges_[i]->removeTetrahedron(this->shared_from_this());
    }
  }
}

template<class T>
bool Tetrahedron<T>::isPointInConvexPosition(const std::array<double, 3>& point,
                                             size_t connecting_triangle_number) const {
  if (!isInfinite()) {
    for (size_t i = 0; i < 4; i++) {
      if (i != connecting_triangle_number) {
        adjacent_triangles_[i]->updatePlaneEquationIfNecessary();
        if (!adjacent_triangles_[i]->trulyOnSameSide(adjacent_nodes_[i]->getPosition(), point))
          return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

template<class T>
int Tetrahedron<T>::isInConvexPosition(const std::array<double, 3>& point,
                                       size_t connecting_triangle_number) const {
  if (!isInfinite()) {
    int result = 1;
    for (size_t i = 0; i < 4; i++) {
      if (i != connecting_triangle_number) {
        adjacent_triangles_[i]->updatePlaneEquationIfNecessary();
        auto position = adjacent_nodes_[i]->getPosition();
        int current_result = adjacent_triangles_[i]->orientation(position, point);
        if (current_result < 0) {
          return -1;
        } else {
          result *= current_result;
        }
      }
    }
    return result;
  } else {
    return -1;
  }
}

template<class T>
std::array<SpaceNode<T>*, 4> Tetrahedron<T>::getAdjacentNodes() const {
  return adjacent_nodes_;
}

template<class T>
std::shared_ptr<Tetrahedron<T> > Tetrahedron<T>::getAdjacentTetrahedron(int number) {
  if (adjacent_triangles_[number] != nullptr) {
    return adjacent_triangles_[number]->getOppositeTetrahedron(this->shared_from_this());
  }
  return std::shared_ptr<Tetrahedron<T> >(nullptr);
}

template<class T>
void Tetrahedron<T>::testPosition(const std::array<double, 3>& position) const
    throw(std::exception) {
  for (auto node : adjacent_nodes_) {
    if (node != nullptr) {
      auto diff = Matrix::subtract(position, node->getPosition());
      if ((std::abs(diff[0]) == 0) && (std::abs(diff[1]) == 0) && (std::abs(diff[2]) == 0)) {
        // TODO throw new PositionNotAllowedException(node.proposeNewPosition());
        // and write swig code to translate it into Java
        throw std::invalid_argument("position not allowed");
      }
    }
  }
}

template<class T>
bool Tetrahedron<T>::isValid() const {
  return valid_;
}

template<class T>
bool Tetrahedron<T>::isNeighbor(const std::shared_ptr<Tetrahedron<T> >& other_tetrahedron) const {
  return (adjacent_triangles_[0]->isAdjacentTo(other_tetrahedron))
      || (adjacent_triangles_[1]->isAdjacentTo(other_tetrahedron))
      || (adjacent_triangles_[2]->isAdjacentTo(other_tetrahedron))
      || (adjacent_triangles_[3]->isAdjacentTo(other_tetrahedron));
}

template<class T>
void Tetrahedron<T>::registerEdges() {
  if (!isInfinite()) {
    auto tetrahedron = std::shared_ptr<Tetrahedron<T>>(nullptr);
    for (size_t i = 0; i < 4; i++) {
      tetrahedron = adjacent_triangles_[i]->getOppositeTetrahedron(this->shared_from_this());
      if ((tetrahedron != nullptr) && (!tetrahedron->isInfinite())) {
        int n1 = tetrahedron->getNodeNumber(adjacent_nodes_[(i + 1) % 4]);
        int n2 = tetrahedron->getNodeNumber(adjacent_nodes_[(i + 2) % 4]);
        int n3 = tetrahedron->getNodeNumber(adjacent_nodes_[(i + 3) % 4]);
        switch (i) {
          case 0:
            adjacent_edges_[3] = tetrahedron->getEdge(n1, n2);
            adjacent_edges_[4] = tetrahedron->getEdge(n1, n3);
            adjacent_edges_[5] = tetrahedron->getEdge(n2, n3);
            break;
          case 1:
            adjacent_edges_[1] = tetrahedron->getEdge(n1, n3);
            adjacent_edges_[2] = tetrahedron->getEdge(n2, n3);
            if (adjacent_edges_[5] == nullptr) {
              adjacent_edges_[5] = tetrahedron->getEdge(n1, n2);
            }
            break;
          case 2:
            adjacent_edges_[0] = tetrahedron->getEdge(n2, n3);
            if (adjacent_edges_[2] == nullptr) {
              adjacent_edges_[2] = tetrahedron->getEdge(n1, n2);
            }
            if (adjacent_edges_[4] == nullptr) {
              adjacent_edges_[4] = tetrahedron->getEdge(n1, n3);
            }
            break;
          case 3:
            if (adjacent_edges_[0] == nullptr) {
              adjacent_edges_[0] = tetrahedron->getEdge(n1, n2);
            }
            if (adjacent_edges_[1] == nullptr) {
              adjacent_edges_[1] = tetrahedron->getEdge(n1, n3);
            }
            if (adjacent_edges_[3] == nullptr) {
              adjacent_edges_[3] = tetrahedron->getEdge(n2, n3);
            }
        }
      }
    }
    // fill up the ones that are missing:
    if (adjacent_nodes_[0] != nullptr) {
      if (adjacent_edges_[0] == nullptr) {
        adjacent_edges_[0] = adjacent_nodes_[0]->searchEdge(adjacent_nodes_[1]);
      }
      if (adjacent_edges_[1] == nullptr) {
        adjacent_edges_[1] = adjacent_nodes_[0]->searchEdge(adjacent_nodes_[2]);
      }
      if (adjacent_edges_[2] == nullptr) {
        adjacent_edges_[2] = adjacent_nodes_[0]->searchEdge(adjacent_nodes_[3]);
      }
    }
    if (adjacent_edges_[3] == nullptr) {
      adjacent_edges_[3] = adjacent_nodes_[1]->searchEdge(adjacent_nodes_[2]);
    }
    if (adjacent_edges_[4] == nullptr) {
      adjacent_edges_[4] = adjacent_nodes_[1]->searchEdge(adjacent_nodes_[3]);
    }
    if (adjacent_edges_[5] == nullptr) {
      adjacent_edges_[5] = adjacent_nodes_[2]->searchEdge(adjacent_nodes_[3]);
    }
    // tell the edges that you are adjacent to it and remember the
    // position where you are inserted:
    for (size_t i = 0; i < 6; i++) {
      if (adjacent_edges_[i] != nullptr) {
        adjacent_edges_[i]->addTetrahedron(this->shared_from_this());
      }
    }
  }
}

template<class T>
void Tetrahedron<T>::changeVolume(double new_volume) {
  double change_per_node = (new_volume - volume_) / 4.0;
  if (change_per_node != 0.0) {
    for (auto node : getAdjacentNodes()) {
      node->changeVolume(change_per_node);
    }
  }
  volume_ = new_volume;
}

template<class T>
std::array<std::array<double, 3>, 3> Tetrahedron<T>::getPlaneNormals() const {
  if (!isInfinite()) {
    auto subtrahend = adjacent_nodes_[0]->getPosition();
    std::array<std::array<double, 3>, 3> ret;
    for (size_t i = 1; i < adjacent_nodes_.size(); i++) {
      ret[i - 1] = Matrix::subtract(adjacent_nodes_[i]->getPosition(), subtrahend);
    }
    return ret;
  }
  throw std::range_error("returning null array not supported");
}

template<class T>
double Tetrahedron<T>::maxAbs(const std::array<std::array<double, 3>, 3>& values) const {
  double ret = 0.0;
  for (size_t i = 0; i < values.size(); i++) {
    for (size_t j = 0; j < values[i].size(); j++) {
      if (std::abs(values[i][j]) > ret) {
        ret = std::abs(values[i][j]);
      }
    }
  }
  return ret;
}

template<class T>
double Tetrahedron<T>::maxAbs(const std::array<double, 3>& values_1,
                              const std::array<double, 3>& values_2,
                              const std::array<double, 3>& values_3,
                              const std::array<double, 3>& values_4) const {
  double ret = 0.0;
  for (size_t i = 0; i < values_1.size(); i++) {
    if (std::abs(values_1[i]) > ret) {
      ret = std::abs(values_1[i]);
    }

    if (std::abs(values_2[i]) > ret) {
      ret = std::abs(values_2[i]);
    }

    if (std::abs(values_3[i]) > ret) {
      ret = std::abs(values_3[i]);
    }

    if (std::abs(values_4[i]) > ret) {
      ret = std::abs(values_4[i]);
    }
  }
  return ret;
}

template<class T>
double Tetrahedron<T>::multError2(double a, double a_err_2, double b, double b_err_2) const {
  return std::max(a_err_2 * b * b + b_err_2 * a * a, 0.00000000001 * a * b * a * b);
}

template<class T>
double Tetrahedron<T>::multError2(double a, double a_err_2, double b, double b_err_2, double c,
                                  double c_err_2) const {
  return std::max(a_err_2 * b * b * c * c + b_err_2 * a * a * c * c + c_err_2 * a * a * b * b,
                  0.00000000001 * a * a * b * b * c * c);
}

template<class T>
double Tetrahedron<T>::addError2(double a_err_2, double b_err_2, double result) const {
  return std::max(b_err_2 + a_err_2, 0.00000000001 * result * result);
}

template<class T>
double Tetrahedron<T>::addError2(double a_err_2, double b_err_2, double c_err_2,
                                 double result) const {
  return std::max(b_err_2 + a_err_2 + c_err_2, 0.00000000001 * result * result);
}

template<class T>
void Tetrahedron<T>::computeCircumsphereCenterAndVolume() {
  auto normals = getPlaneNormals();
  changeVolume(std::abs(Matrix::det(normals)) / 6.0);

  double nm = maxAbs(normals);
  double max_length_2 = 0.0;
  // normalize normal-vectors
  for (int i = 0; i < 3; i++) {
    double length = normals[i][0] * normals[i][0] + normals[i][1] * normals[i][1]
        + normals[i][2] * normals[i][2];
    if (length > max_length_2) {
      max_length_2 = length;
    }
    length = MathUtil::sqrt(length);

    normals[i][0] /= length;
    normals[i][1] /= length;
    normals[i][2] /= length;
  }
  double my_2 = 0.000000000000001;
  // max squared error of the new normal value / my2
  double tmp = nm * nm * (1 / max_length_2 + 1 / (max_length_2 * max_length_2));
  double dns_2 = std::max(1.0, tmp);
  double ddet_2 = 36 * dns_2;

  double pm_2 = maxAbs(adjacent_nodes_[0]->getPosition(), adjacent_nodes_[1]->getPosition(),
                       adjacent_nodes_[2]->getPosition(), adjacent_nodes_[3]->getPosition());
  pm_2 *= pm_2;
  // Offset-Error / My2
  double doff_2 = 6 * pm_2 * (dns_2 + 1);
  double dscalar_2 = 4 * doff_2 + 36 * pm_2 * dns_2;
  double ddiv_2 = 0.0;
  double det = Matrix::det(normals);

  auto add_01 = Matrix::add(adjacent_nodes_[0]->getPosition(), adjacent_nodes_[1]->getPosition());
  auto add_02 = Matrix::add(adjacent_nodes_[0]->getPosition(), adjacent_nodes_[2]->getPosition());
  auto add_03 = Matrix::add(adjacent_nodes_[0]->getPosition(), adjacent_nodes_[3]->getPosition());
  std::array<double, 3> offsets;
  offsets[0] = 0.5 * Matrix::dot(normals[0], add_01);
  offsets[1] = 0.5 * Matrix::dot(normals[1], add_02);
  offsets[2] = 0.5 * Matrix::dot(normals[2], add_03);

  circum_center_ = Triangle3D<T>::calculate3PlaneXPoint(normals, offsets, det);
  circum_center_is_null_ = false;

  if (det != 0) {
    ddiv_2 = 1 / (det * det) * 3 * dscalar_2 + 324 * pm_2 * ddet_2 / (det * det * det * det);
    auto dummy = Matrix::subtract(circum_center_, adjacent_nodes_[0]->getPosition());
    squared_radius_ = Matrix::dot(dummy, dummy);
    tolerance_ = MathUtil::sqrt(12 * ddiv_2 * squared_radius_) * my_2;
  }
  updateCrossSectionAreas();
}

template<class T>
void Tetrahedron<T>::computeRadius() {
  auto dummy = Matrix::subtract(circum_center_, adjacent_nodes_[0]->getPosition());
  squared_radius_ = Matrix::dot(dummy, dummy);
}

template<class T>
SpaceNode<T>* Tetrahedron<T>::getFirstOtherNode(
    SpaceNode<T>* node_a,
    SpaceNode<T>* node_b) const {
  for (auto node : adjacent_nodes_) {
    if (node != node_a && node != node_b) {
      return node;
    }
  }
  return nullptr;
}

template<class T>
SpaceNode<T>* Tetrahedron<T>::getSecondOtherNode(
    SpaceNode<T>* node_a,
    SpaceNode<T>* node_b) const {
  for (int i = adjacent_nodes_.size() - 1; i >= 0; i--) {
    if (adjacent_nodes_[i] != node_a && adjacent_nodes_[i] != node_b) {
      return adjacent_nodes_[i];
    }
  }
  return nullptr;
}

template<class T>
Tetrahedron<T>::Tetrahedron()
    : adjacent_nodes_(),
      adjacent_edges_(),
      adjacent_triangles_(),
      cross_section_areas_ { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
      circum_center_ { 0.0, 0.0, 0.0 },
      circum_center_is_null_(true),
      squared_radius_(0.0),
      tolerance_(0.0000001),
      volume_(0.0),
      valid_(true) {
}

template<class T>
void Tetrahedron<T>::initializationHelper(const std::shared_ptr<Triangle3D<T>>& one_triangle,
                                          SpaceNode<T>* fourth_point,
                                          const std::shared_ptr<OpenTriangleOrganizer<T>>& oto) {
  auto triangle = one_triangle;
  auto point = fourth_point;
  if (triangle->isInfinite()) {
    auto nodes = triangle->getNodes();
    auto space_node_a = (nodes[0] == nullptr) ? nodes[1] : nodes[0];
    auto space_node_b = (nodes[2] == nullptr) ? nodes[1] : nodes[2];
    triangle = oto->getTriangleWithoutRemoving(space_node_a, space_node_b, point);
    point = nullptr;
  }
  adjacent_nodes_[0] = point;
  if (point != nullptr) {
    point->addTetrahedron(this->shared_from_this());
  }
  auto triangle_nodes = triangle->getNodes();
  for (size_t i = 0; i < triangle_nodes.size(); i++) {
    adjacent_nodes_[i + 1] = triangle_nodes[i];
    triangle_nodes[i]->addTetrahedron(this->shared_from_this());
  }
  // add triangle and make sure that adjacent_triangles_[i] lies opposite to
  // adjacent_nodes_[i]:
  adjacent_triangles_[0] = triangle;
  if (!triangle->isCompletelyOpen()) {
    oto->removeTriangle(triangle);
  }
  adjacent_triangles_[1] = oto->getTriangle(point, triangle_nodes[1], triangle_nodes[2]);
  adjacent_triangles_[2] = oto->getTriangle(point, triangle_nodes[0], triangle_nodes[2]);
  adjacent_triangles_[3] = oto->getTriangle(point, triangle_nodes[0], triangle_nodes[1]);
  for (size_t i = 0; i < 4; i++) {
    adjacent_triangles_[i]->addTetrahedron(this->shared_from_this());
  }
  registerEdges();
  calculateCircumSphere();
}

template<class T>
void Tetrahedron<T>::initializationHelper(const std::shared_ptr<Triangle3D<T>>& triangle_a,
                                          const std::shared_ptr<Triangle3D<T>>& triangle_b,
                                          const std::shared_ptr<Triangle3D<T>>& triangle_c,
                                          const std::shared_ptr<Triangle3D<T>>& triangle_d,
                                          SpaceNode<T>* node_a,
                                          SpaceNode<T>* node_b,
                                          SpaceNode<T>* node_c,
                                          SpaceNode<T>* node_d) {
  adjacent_triangles_[0] = triangle_a;
  adjacent_triangles_[1] = triangle_b;
  adjacent_triangles_[2] = triangle_c;
  adjacent_triangles_[3] = triangle_d;
  adjacent_nodes_[0] = node_a;
  adjacent_nodes_[1] = node_b;
  adjacent_nodes_[2] = node_c;
  adjacent_nodes_[3] = node_d;

  adjacent_triangles_[0]->addTetrahedron(this->shared_from_this());
  if (adjacent_nodes_[0] != nullptr) {
    adjacent_nodes_[0]->addTetrahedron(this->shared_from_this());
  }
  for (size_t i = 1; i < 4; i++) {
    adjacent_triangles_[i]->addTetrahedron(this->shared_from_this());
    adjacent_nodes_[i]->addTetrahedron(this->shared_from_this());
  }
  registerEdges();
  calculateCircumSphere();
}

template class Tetrahedron<bdm::physics::PhysicalNode>;

}  // namespace spatial_organization
}  // namespace bdm
