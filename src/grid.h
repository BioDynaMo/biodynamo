#ifndef GRID_H_
#define GRID_H_

#include <iostream>
#include <array>
#include <cmath>
#include <limits>
#include <vector>
#include "inline_vector.h"

namespace bdm {

using std::array;
using std::vector;
using std::fmod;

class Grid {
 public:
  struct Box {
    Grid* grid_;
    uint64_t start_ = 0;
    uint16_t length_ = 0;
    bool is_initialized_ = false;

    explicit Box(Grid* grid, bool is_initialized = true) : grid_(grid), is_initialized_(is_initialized) {}

    bool IsEmpty() const { return length_ == 0; }

    bool IsInitialized() const { return is_initialized_; }

    void AddCell(size_t cell_id) {
      if (IsEmpty()) {
        start_ = cell_id;
      } else {
        // add to front
        grid_->successors_[cell_id] = start_;
        start_ = cell_id;
      }
      length_++;
    }

    // TODO remove cell

    struct Iterator {
      Iterator(Grid* grid, const Box* box)
          : grid_(grid),
            current_value_(box->start_),
            countdown_(box->length_) {}

      bool IsAtEnd() { return countdown_ <= 0; }

      Iterator operator++() {
        countdown_--;
        current_value_ = grid_->successors_[current_value_];
        return *this;
      }

      size_t operator*() const { return current_value_; }

      Grid* grid_;
      size_t current_value_;
      int countdown_;
    };

    Iterator begin() const { return Iterator(grid_, this); }
  };

  struct NeighborIterator {
    NeighborIterator(InlineVector<const Box*, 27>* neighbor_boxes)
        : neighbor_boxes_(neighbor_boxes),
          box_iterator_((*neighbor_boxes_)[0]->begin()) {
      // if first box is empty
      if ((*neighbor_boxes_)[0]->IsEmpty()) {
        ForwardToNonEmptyBox();
      }
    }

    bool IsAtEnd() const { return is_end_; }

    size_t operator*() const { return *box_iterator_; }

    /// version where empty neighbors in neighbor_boxes_ are allowed
    NeighborIterator& operator++() {
      ++box_iterator_;
      // if iterator of current box has come to an end, continue with next box
      if (box_iterator_.IsAtEnd()) {
        return ForwardToNonEmptyBox();
      }
      return *this;
    }

   private:
    InlineVector<const Box*, 27>* neighbor_boxes_;
    Box::Iterator box_iterator_;
    uint16_t box_idx_ = 0;
    bool is_end_ = false;

    /// Forwards the iterator to the next non empty box and returns itself
    /// If there are no non empty boxes is_end_ is set to true
    NeighborIterator& ForwardToNonEmptyBox() {
      // increment box id until non empty box has been found
      while (++box_idx_ < neighbor_boxes_->size()) {
        // box is empty or uninitialized (padding box) -> continue
        if (!((*neighbor_boxes_)[box_idx_]->IsInitialized()) ||
              (*neighbor_boxes_)[box_idx_]->IsEmpty()) {
          continue;
        }
        // a non-empty box has been found
        box_iterator_ = (*neighbor_boxes_)[box_idx_]->begin();
        return *this;
      }
      // all remaining boxes have been empty; reached end
      is_end_ = true;
      return *this;
    }
  };

  /// Enum that determines the degree of adjacency in search neighbor boxes
  /// LOW:    The closest 8  neighboring boxes
  /// MEDIUM: The closest 18 neighboring boxes
  /// HIGH:   The closest 26 neighboring boxes
  enum Adjacency {LOW, MEDIUM, HIGH};

  Grid(vector<array<double, 3>>& positions, uint32_t box_length, Adjacency adjacency = HIGH)
      : positions_(positions), box_length_(box_length), adjacency_(adjacency) {
    UpdateGrid(positions);
  }

  template <typename TContainer>
  Grid(TContainer* cells, uint32_t box_length, Adjacency adjacency = HIGH)
          : positions_(cells->GetAllPositions()), box_length_(box_length), adjacency_(adjacency) {
    UpdateGrid(cells->GetAllPositions());
  }

  virtual ~Grid() { delete empty_box_; }

  /// Clears the grid 
  void ClearGrid() {
    boxes_.clear();
    num_boxes_axis_ = {{0}};
    num_boxes_xy_ = 0;
    successors_.clear();
    delete empty_box_;
  }

  /// Updates the grid, as simulation objects may have moved, added or deleted
  void UpdateGrid(vector<array<double, 3>>& positions) {
    ClearGrid();
    auto grid_dimensions = CalculateGridDimensions(positions);
    for (int i = 0; i < 3; i++) {
      double dimension_length = grid_dimensions[2*i + 1] - grid_dimensions[2*i];
      double r = fmod(dimension_length, box_length_);
      // If the grid is not perfectly divisible along each dimension by the 
      // resolution, extend the grid so that it is
      if (r != 0.0) {
        grid_dimensions[2*i + 1] += dimension_length - box_length_;
      } else {
        // Else extend the grid dimension with one row, because the outmost cell
        // lies exactly on the border
        grid_dimensions[2*i + 1] += box_length_;
      }
    }

    // Pad the grid to avoid out of bounds check when search neighbors
    for (int i = 0; i < 3; i++) {
      grid_dimensions[2*i] -= box_length_;
      grid_dimensions[2*i + 1] += box_length_;
    }

    // Calculate how many boxes fit along each dimension
    for (int i = 0; i < 3; i++) {
      double dimension_length = grid_dimensions[2*i + 1] - grid_dimensions[2*i];
      while (dimension_length > 0.0) {
        dimension_length -= box_length_;
        num_boxes_axis_[i]++;
      }
    }

    num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];
    auto total_num_boxes = num_boxes_xy_ * num_boxes_axis_[2];

    boxes_.resize(total_num_boxes, Box(this));
    empty_box_ = new Box(this, false);

    // initialize successors_;
    successors_.resize(positions_.size());

    // assign simulation objects to boxes
    for (size_t cell_id = 0; cell_id < positions_.size(); cell_id++) {
      auto& position = positions_[cell_id];
      auto box = GetBoxPointer(GetBoxIndex(position));
      box->AddCell(cell_id);
    }
  }

  /// Calculates what the grid dimensions need to be in order to contain
  /// all the simulation objects
  array<double, 6> CalculateGridDimensions(vector<array<double, 3>>& positions) {
    array<double, 6> grid_dimensions = {{1e15, 0, 1e15, 0, 1e15, 0}};
    for (size_t i = 0; i < positions.size(); i++) {
      auto position = positions[i];
      for (size_t j = 0; j < 3; j++) {
        if (position[j] < grid_dimensions[2*j]) {
          grid_dimensions[2*j] = position[j];
        }
        if (position[j] > grid_dimensions[2*j + 1]) {
          grid_dimensions[2*j + 1] = position[j];
        }
      }
    }
    return grid_dimensions;
  }

  template<typename Lambda>
  void ForEachNeighbor(Lambda lambda) {
    vector<size_t> sum(positions_.size());
#pragma omp parallel for
    // qc = query cell
    for (size_t qc = 0; qc < positions_.size(); qc++) {
      auto& position = positions_[qc];
      auto idx = GetBoxIndex(position);

      InlineVector<const Box*, 27> neighbor_boxes;
      GetMooreBoxes(&neighbor_boxes, idx);

      NeighborIterator ni(&neighbor_boxes);
      while (!ni.IsAtEnd()) {
        // do something with nc
        // std::cout << *ni << " ";
        lambda(*ni, qc);
        ++ni;
      }
      // std::cout << std::endl;
    }
  }

  inline double SquaredEuclideanDistance(std::array<double, 3> pos1, std::array<double, 3> pos2) const {
    const double dx = pos2[0] - pos1[0];
    const double dy = pos2[1] - pos1[1];
    const double dz = pos2[2] - pos1[2];
    return (dx * dx + dy * dy + dz * dz);
  }

  template<typename TContainer>
  void SetNeighborsWithinRadius(TContainer* sim_objects, double distance) {
    vector<size_t> sum(positions_.size());
    InlineVector<int, 8> neighbors;
#pragma omp parallel for firstprivate(neighbors)
    // qc = query cell
    for (size_t qc = 0; qc < positions_.size(); qc++) {
      auto& position = positions_[qc];
      auto idx = GetBoxIndex(position);

      InlineVector<const Box*, 27> neighbor_boxes;
      GetMooreBoxes(&neighbor_boxes, idx);

      NeighborIterator ni(&neighbor_boxes);
      neighbors.clear();
      while (!ni.IsAtEnd()) {
        if (*ni != qc) {
          if (SquaredEuclideanDistance(positions_[qc], positions_[*ni]) < distance) {
            neighbors.push_back(*ni);
          }
        }
        ++ni;
      }
      (*sim_objects)[qc].SetNeighbors(neighbors);
    }
  }

 private:
  vector<Box> boxes_;
  vector<array<double, 3>>& positions_;
  /// Length of a Box
  uint32_t box_length_ = 0;
  /// Stores the number of boxes for each axis
  array<uint32_t, 3> num_boxes_axis_ = {{0}};
  /// Number of boxes in the xy plane (=num_boxes_axis_[0] * num_boxes_axis_[1])
  size_t num_boxes_xy_ = 0;
  /// Implements linked list - array index = key, value: next element
  vector<size_t> successors_;
  /// Determines which boxes to search neighbors in (see enum Adjacency)
  Adjacency adjacency_;
  /// An empty box that will be used to initialize the pad boxes
  Box* empty_box_ = nullptr;

  void GetMooreBoxes(InlineVector<const Box*, 27>* neighbor_boxes,
                     size_t box_idx) {
    neighbor_boxes->push_back(GetBoxPointer(box_idx));

    // Adjacent 6 (top, down, left, right, front and back)
    if (adjacency_ >= LOW) {
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + 1));
    }

    // Adjacent 12
    if (adjacency_ >= MEDIUM) {
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ - 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ - 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ + 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[1] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ + 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[1] + 1));
    }

    // Adjacent 8
    if (adjacency_ >= HIGH) {
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[1] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[1] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[1] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[1] + 1));
    }
  }

  const Box* GetBoxPointer(size_t index) const {
    if (index < boxes_.size()) {
      return &(boxes_[index]);
    } else {
      return empty_box_;
    }
  }

  Box* GetBoxPointer(size_t index) {
    if (index < boxes_.size()) {
      return &(boxes_[index]);
    } else {
      return empty_box_;
    }
  }

  /// Returns the box coordinates based on the box index in the one dimensional
  /// array
  /// @param box_idx - index of the box in boxes_
  array<uint32_t, 3> GetBoxCoordinates(size_t box_idx) const {
    array<uint32_t, 3> ret;
    ret[2] = box_idx / num_boxes_xy_;
    auto xy = box_idx % num_boxes_xy_;
    ret[1] = xy / num_boxes_axis_[0];
    ret[0] = xy % num_boxes_axis_[0];
    return ret;
  }

  /// Returns the box index in the one dimensional array based on
  /// box coordinates in space
  /// @param box_coord - box coordinates in space (x, y, z)
  size_t GetBoxIndex(array<uint32_t, 3>& box_coord) const {
    // z * num_boxes_xy_ + y * num_boxes_x + x
    return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] +
           box_coord[0];
  }

  /// Return the box index in the one dimensional array of the box that contains
  /// the position
  size_t GetBoxIndex(const array<double, 3> position) {
    array<uint32_t, 3> box_coord;
    box_coord[0] = floor(position[0]) / box_length_;
    box_coord[1] = floor(position[1]) / box_length_;
    box_coord[2] = floor(position[2]) / box_length_;

    return GetBoxIndex(box_coord);
  }
};

}  // namespace bdm

#endif  // GRID_H_
