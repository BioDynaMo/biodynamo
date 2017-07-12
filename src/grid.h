#ifndef GRID_H_
#define GRID_H_

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include "inline_vector.h"

namespace bdm {

using std::array;
using std::vector;
using std::fmod;

///
/// @brief      A class that represents Cartesian 3D grid
///
class Grid {
 public:
  ///
  /// @brief      A single unit cube of the grid
  ///
  struct Box {
    Grid* grid_;
    uint64_t start_ = 0;
    uint16_t length_ = 0;
    bool is_initialized_ = false;

    explicit Box(Grid* grid, bool is_initialized = true)
        : grid_(grid), is_initialized_(is_initialized) {}

    bool IsEmpty() const { return length_ == 0; }

    bool IsInitialized() const { return is_initialized_; }

    ///
    /// @brief      Adds a simulation object to this box
    ///
    /// @param[in]  obj_id  The object's identifier
    ///
    void AddObject(size_t obj_id) {
      if (IsEmpty()) {
        start_ = obj_id;
      } else {
        // Add to the linked list of successor cells
        grid_->successors_[obj_id] = start_;
        start_ = obj_id;
      }
      length_++;
    }

    ///
    /// @brief      An iterator that iterates over the cells in this box
    ///
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

    Iterator begin() const { return Iterator(grid_, this); }  // NOLINT
  };

  ///
  /// @brief      An iterator that iterates over the boxes in this grid
  ///
  struct NeighborIterator {
    explicit NeighborIterator(InlineVector<const Box*, 27>* neighbor_boxes)
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

  ///
  /// @brief      Enum that determines the degree of adjacency in search
  /// neighbor
  ///             boxes
  ///
  enum Adjacency {
    kLow,    /**< The closest 8  neighboring boxes */
    kMedium, /**< The closest 18  neighboring boxes */
    kHigh    /**< The closest 26  neighboring boxes */
  };

  Grid() {}

  Grid(Grid const&) = delete;
  void operator=(Grid const&) = delete;

  ///
  /// @brief      Initialize the grid with the given object positions
  ///
  /// @param      positions   The positions of the objects
  /// @param[in]  box_length  The box length
  /// @param[in]  adjacency   The adjacency (see #Adjacency)
  ///
  void Initialize(vector<array<double, 3>>* positions,
                  uint32_t box_length,  // NOLINT
                  Adjacency adjacency = kHigh) {
    positions_ = positions;
    box_length_ = box_length;
    adjacency_ = adjacency;

    UpdateGrid(*positions);
  }

  ///
  /// @brief      Initialize the grid with the given simulation objects
  ///
  /// @param      sim_objects  The simulation objects
  /// @param[in]  box_length   The box length
  /// @param[in]  adjacency    The adjacency (see #Adjacency)
  ///
  /// @tparam     TContainer   { description }
  ///
  template <typename TContainer>
  void Initialize(TContainer* sim_objects, uint32_t box_length,  // NOLINT
                  Adjacency adjacency = kHigh) {
    positions_ = &(sim_objects->GetAllPositions());
    box_length_ = box_length;
    adjacency_ = adjacency;

    UpdateGrid(sim_objects->GetAllPositions());
  }

  virtual ~Grid() { delete empty_box_; }

  ///
  /// @brief      Gets the singleton instance
  ///
  /// @return     The instance
  ///
  static Grid& GetInstance() {
    static Grid kGrid;
    return kGrid;
  }

  ///
  /// @brief      Clears the grid
  ///
  void ClearGrid() {
    boxes_.clear();
    num_boxes_axis_ = {{0}};
    num_boxes_xy_ = 0;

    successors_.clear();
    delete empty_box_;
  }

  ///
  /// @brief      Updates the grid, as simulation objects may have moved, added
  ///             or deleted
  ///
  /// @param      positions  The updated positions of the simulation objects
  ///
  void UpdateGrid(vector<array<double, 3>>& positions) {  // NOLINT
    ClearGrid();
    auto grid_dimensions = CalculateGridDimensions(positions);
    for (int i = 0; i < 3; i++) {
      double dimension_length =
          grid_dimensions[2 * i + 1] - grid_dimensions[2 * i];
      double r = fmod(dimension_length, box_length_);
      // If the grid is not perfectly divisible along each dimension by the
      // resolution, extend the grid so that it is
      if (r != 0.0) {
        grid_dimensions[2 * i + 1] += dimension_length - box_length_;
      } else {
        // Else extend the grid dimension with one row, because the outmost
        // object
        // lies exactly on the border
        grid_dimensions[2 * i + 1] += box_length_;
      }
    }

    // Pad the grid to avoid out of bounds check when search neighbors
    for (int i = 0; i < 3; i++) {
      grid_dimensions[2 * i] -= box_length_;
      grid_dimensions[2 * i + 1] += box_length_;
    }

    // Calculate how many boxes fit along each dimension
    for (int i = 0; i < 3; i++) {
      double dimension_length =
          grid_dimensions[2 * i + 1] - grid_dimensions[2 * i];
      while (dimension_length > 0.0) {
        dimension_length -= box_length_;
        num_boxes_axis_[i]++;
      }
    }

    num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];
    auto total_num_boxes = num_boxes_xy_ * num_boxes_axis_[2];

    boxes_.resize(total_num_boxes, Box(this));

    // Create an empty box to initialize the padding boxes with
    empty_box_ = new Box(this, false);

    // Initialize successors_;
    successors_.resize(positions.size());

    // Assign simulation objects to boxes
    for (size_t obj_id = 0; obj_id < positions.size(); obj_id++) {
      auto& position = positions[obj_id];
      auto box = GetBoxPointer(GetBoxIndex(position));
      box->AddObject(obj_id);
    }
  }

  ///
  /// Calculates what the grid dimensions need to be in order to contain /
  /// all the simulation objects
  ///
  /// @param      positions  The positions of the simulation objects
  ///
  /// @return     The grid dimensions
  ///
  array<double, 6> CalculateGridDimensions(
      vector<array<double, 3>>& positions) {  // NOLINT
    array<double, 6> grid_dimensions = {{1e15, 0, 1e15, 0, 1e15, 0}};
    for (size_t i = 0; i < positions.size(); i++) {
      auto position = positions[i];
      for (size_t j = 0; j < 3; j++) {
        if (position[j] < grid_dimensions[2 * j]) {
          grid_dimensions[2 * j] = position[j];
        }
        if (position[j] > grid_dimensions[2 * j + 1]) {
          grid_dimensions[2 * j + 1] = position[j];
        }
      }
    }
    return grid_dimensions;
  }

  ///
  /// @brief      Apply an operation for all the neighbor objects on each
  ///             simulation object
  ///
  /// @param[in]  lambda  The operation as a lambda
  ///
  template <typename Lambda>
  void ForEachNeighbor(Lambda lambda) {
#pragma omp parallel for
    // q = query object
    for (size_t q = 0; q < positions_->size(); q++) {
      auto& position = (*positions_)[q];
      auto idx = GetBoxIndex(position);

      InlineVector<const Box*, 27> neighbor_boxes;
      GetMooreBoxes(&neighbor_boxes, idx);

      NeighborIterator ni(&neighbor_boxes);
      while (!ni.IsAtEnd()) {
        // do something with neighbor object
        lambda(*ni, q);
        ++ni;
      }
    }
  }

  ///
  /// @brief      Calculates the squared euclidian distance between two points
  ///             in 3D
  ///
  /// @param[in]  pos1  Position of the first point
  /// @param[in]  pos2  Position of the second point
  ///
  /// @return     The distance between the two points
  ///
  inline double SquaredEuclideanDistance(std::array<double, 3> pos1,
                                         std::array<double, 3> pos2) const {
    const double dx = pos2[0] - pos1[0];
    const double dy = pos2[1] - pos1[1];
    const double dz = pos2[2] - pos1[2];
    return (dx * dx + dy * dy + dz * dz);
  }

  ///
  /// @brief      { function_description }
  ///
  /// @param[in]  lambda  The lambda
  /// @param      query   The query
  /// @param[in]  radius  The radius
  ///
  /// @tparam     Lambda  { description }
  /// @tparam     SO      { description }
  ///
  template <typename Lambda, typename SO>
  void ForEachNeighborWithinRadius(Lambda lambda, SO* query, double radius) {
    auto& position = query->GetPosition();
    auto idx = GetBoxIndex(position);

    InlineVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    NeighborIterator ni(&neighbor_boxes);
    while (!ni.IsAtEnd()) {
      // do something with neighbor object
      if (*ni != query->id()) {
        if (SquaredEuclideanDistance(position, (*positions_)[*ni]) < radius) {
          lambda(*ni);
        }
      }
      ++ni;
    }
  }

  ///
  /// @brief      Sets the neighbors of each object found within a radius
  ///
  /// @param      sim_objects  The simulation objects
  /// @param[in]  radius       The search radius
  ///
  /// @tparam     TContainer   The container type that holds the simulation
  ///                          objects
  ///
  template <typename TContainer>
  void SetNeighborsWithinRadius(TContainer* sim_objects, double radius) {
    vector<size_t> sum(positions_->size());
    InlineVector<int, 8> neighbors;
#pragma omp parallel for firstprivate(neighbors)
    // q = query object
    for (size_t q = 0; q < positions_->size(); q++) {
      auto& position = (*positions_)[q];
      auto idx = GetBoxIndex(position);

      InlineVector<const Box*, 27> neighbor_boxes;
      GetMooreBoxes(&neighbor_boxes, idx);

      NeighborIterator ni(&neighbor_boxes);
      neighbors.clear();
      while (!ni.IsAtEnd()) {
        if (*ni != q) {
          if (SquaredEuclideanDistance((*positions_)[q], (*positions_)[*ni]) <
              radius) {
            neighbors.push_back(*ni);
          }
        }
        ++ni;
      }
      (*sim_objects)[q].SetNeighbors(neighbors);
    }
  }

 private:
  vector<Box> boxes_;
  vector<array<double, 3>>* positions_ = nullptr;
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

  ///
  /// @brief      Gets the Moore (i.e adjacent) boxes of the query box
  ///
  /// @param      neighbor_boxes  The neighbor boxes
  /// @param[in]  box_idx         The query box
  ///
  void GetMooreBoxes(InlineVector<const Box*, 27>* neighbor_boxes,
                     size_t box_idx) {
    neighbor_boxes->push_back(GetBoxPointer(box_idx));

    // Adjacent 6 (top, down, left, right, front and back)
    if (adjacency_ >= kLow) {
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + 1));
    }

    // Adjacent 12
    if (adjacency_ >= kMedium) {
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_axis_[1] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_axis_[1] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[1]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_axis_[1] + 1));
    }

    // Adjacent 8
    if (adjacency_ >= kHigh) {
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

  ///
  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  const Box* GetBoxPointer(size_t index) const {
    if (index < boxes_.size()) {
      return &(boxes_[index]);
    } else {
      return empty_box_;
    }
  }

  ///
  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  Box* GetBoxPointer(size_t index) {
    if (index < boxes_.size()) {
      return &(boxes_[index]);
    } else {
      return empty_box_;
    }
  }

  /// Returns the box index in the one dimensional array based on box
  /// coordinates in space
  ///
  /// @param      box_coord  box coordinates in space (x, y, z)
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(array<uint32_t, 3>& box_coord) const {  // NOLINT
    return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] +
           box_coord[0];
  }

  ///
  /// @brief      Return the box index in the one dimensional array of the box
  ///             that contains the position
  ///
  /// @param[in]  position  The position of the object
  ///
  /// @return     The box index.
  ///
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
