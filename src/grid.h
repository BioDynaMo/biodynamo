#ifndef GRID_H_
#define GRID_H_

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <omp.h>
#include "cell.h"
#include "inline_vector.h"
#include "param.h"

namespace bdm {

using std::array;
using std::vector;
using std::fmod;

/// Vector with fixed number of elements == Array with push_back function that
/// keeps track of its size
/// NB: No bounds checking. Do not push_back more often than the number of
/// maximum elements given by the template parameter N
template <typename T, std::size_t N>
class FixedSizeVector {
 public:
  size_t size() const { return size_; }  // NOLINT

  const T& operator[](size_t idx) const { return data_[idx]; }

  T& operator[](size_t idx) { return data_[idx]; }

  void push_back(const T& value) { data_[size_++] = value; }  // NOLINT

  void clear() { size_ = 0; }

  FixedSizeVector& operator++() {
    #pragma omp simd
    for (size_t i = 0; i < N; i++) {
      ++data_[i];
    }
    return *this;
  }

 private:
  T data_[N] __attribute__ ((aligned (64)));
  std::size_t size_ = 0;
};

/// A class that represents Cartesian 3D grid
class Grid {
 public:
  /// A single unit cube of the grid
  struct Box {
    uint64_t start_ = 0;
    uint16_t length_ = 0;

    Box() {}

    bool IsEmpty() const { return length_ == 0; }

    /// @brief      Adds a simulation object to this box
    ///
    /// @param[in]  obj_id  The object's identifier
    ///
    void AddObject(size_t obj_id) {
      if (IsEmpty()) {
        start_ = obj_id;
      } else {
        // Add to the linked list of successor cells
        Grid::GetInstance().successors_[obj_id] = start_;
        start_ = obj_id;
      }
      length_++;
    }

    /// An iterator that iterates over the cells in this box
    struct Iterator {
      Iterator(Grid* grid, const Box* box)
          : grid_(grid),
            current_value_(box->start_),
            countdown_(box->length_) {}

      bool IsAtEnd() { return countdown_ <= 0; }

      Iterator& operator++() {
        countdown_--;
        current_value_ = grid_->successors_[current_value_];
        return *this;
      }

      size_t operator*() const { return current_value_; }

      Grid* grid_;
      size_t current_value_;
      int countdown_;
    };

    Iterator begin() const {  // NOLINT
      return Iterator(&(Grid::GetInstance()), this);
    }
  };

  /// An iterator that iterates over the boxes in this grid
  struct NeighborIterator {
    explicit NeighborIterator(
        const FixedSizeVector<const Box*, 27>& neighbor_boxes)
        : neighbor_boxes_(neighbor_boxes),
          box_iterator_(neighbor_boxes_[0]->begin()) {
      // if first box is empty
      if (neighbor_boxes_[0]->IsEmpty()) {
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
    const FixedSizeVector<const Box*, 27>& neighbor_boxes_;
    Box::Iterator box_iterator_;
    uint16_t box_idx_ = 0;
    bool is_end_ = false;

    /// Forwards the iterator to the next non empty box and returns itself
    /// If there are no non empty boxes is_end_ is set to true
    NeighborIterator& ForwardToNonEmptyBox() {
      // increment box id until non empty box has been found
      while (++box_idx_ < neighbor_boxes_.size()) {
        // box is empty or uninitialized (padding box) -> continue
        if (neighbor_boxes_[box_idx_]->IsEmpty()) {
          continue;
        }
        // a non-empty box has been found
        box_iterator_ = neighbor_boxes_[box_idx_]->begin();
        return *this;
      }
      // all remaining boxes have been empty; reached end
      is_end_ = true;
      return *this;
    }
  };

  /// Enum that determines the degree of adjacency in search neighbor boxes
  enum Adjacency {
    kLow,    /**< The closest 8  neighboring boxes */
    kMedium, /**< The closest 18  neighboring boxes */
    kHigh    /**< The closest 26  neighboring boxes */
  };

  Grid() {}

  Grid(Grid const&) = delete;
  void operator=(Grid const&) = delete;

  /// @brief      Initialize the grid with the given simulation objects
  ///
  /// @param      sim_objects  The simulation objects
  /// @param[in]  adjacency    The adjacency (see #Adjacency)
  ///
  /// @tparam     TContainer   The container type that holds the simulation
  ///                          objects
  ///
  template <typename TContainer>
  void Initialize(const TContainer& sim_objects, Adjacency adjacency = kHigh) {
    adjacency_ = adjacency;

    UpdateGrid(sim_objects);
  }

  virtual ~Grid() {}

  /// @brief      Gets the singleton instance
  ///
  /// @return     The instance
  ///
  static Grid& GetInstance() {
    static Grid kGrid;
    return kGrid;
  }

  /// Clears the grid
  void ClearGrid() {
    boxes_.clear();
    box_length_ = 0;
    largest_object_size_ = 0;
    num_boxes_axis_ = {{0}};
    num_boxes_xy_ = 0;
    auto inf = Param::kInfinity;
    grid_dimensions_ = {inf, -inf, inf, -inf, inf, -inf};
    successors_.clear();
  }

  /// @brief      Updates the grid, as simulation objects may have moved, added
  ///             or deleted
  ///
  /// @param      sim_objects  The simulation objects
  ///
  /// @tparam     TContainer   The container type that holds the simulation
  ///                          objects
  ///
  template <typename TContainer>
  void UpdateGrid(const TContainer& sim_objects) {
    ClearGrid();
    CalculateGridDimensions(sim_objects);
    // todo: in some cases smaller box length still gives correct simulation
    // results (and is faster). Find out what this should be set to
    box_length_ = ceil(largest_object_size_);
    box_length_squared_ = box_length_ * box_length_;
    for (int i = 0; i < 3; i++) {
      double dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      double r = fmod(dimension_length, box_length_);
      // If the grid is not perfectly divisible along each dimension by the
      // resolution, extend the grid so that it is
      if (r != 0.0) {
        grid_dimensions_[2 * i + 1] += dimension_length - box_length_;
      } else {
        // Else extend the grid dimension with one row, because the outmost
        // object
        // lies exactly on the border
        grid_dimensions_[2 * i + 1] += box_length_;
      }
    }

    // Pad the grid to avoid out of bounds check when search neighbors
    for (int i = 0; i < 3; i++) {
      grid_dimensions_[2 * i] -= box_length_;
      grid_dimensions_[2 * i + 1] += box_length_;
    }

    // Calculate how many boxes fit along each dimension
    for (int i = 0; i < 3; i++) {
      double dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      while (dimension_length > 0.0) {
        dimension_length -= box_length_;
        num_boxes_axis_[i]++;
      }
    }

    num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];
    auto total_num_boxes = num_boxes_xy_ * num_boxes_axis_[2];

    boxes_.resize(total_num_boxes, Box());

    // Initialize successors_;
    successors_.resize(sim_objects.size());

    // Assign simulation objects to boxes
    for (size_t i = 0; i < sim_objects.size(); i++) {
      const auto& position = sim_objects[i].GetPosition();
      auto box = GetBoxPointer(GetBoxIndex(position));
      box->AddObject(i);  // i = simulation object id
    }

    size_t num_cells = 0;
    size_t non_empty_boxes = 0;
    for (size_t i = 0; i < boxes_.size(); i++) {
      num_cells += boxes_[i].length_;
      if (boxes_[i].length_ != 0) {
        non_empty_boxes++;
      }
    }
    // std::cout << "total number of boxes:       " << total_num_boxes << std::endl;
    // std::cout << "non empty boxes:             " << non_empty_boxes << std::endl;
    // std::cout << "cells inboxes:               " << num_cells << std::endl;
    // std::cout << "avg number of cells per box: " << num_cells / non_empty_boxes << std::endl;
    // for(uint32_t z = 1; z < num_boxes_axis_[2] -1 ; z++) {
    //   for(uint32_t y = 1; y < num_boxes_axis_[1] -1; y++) {
    //       for(uint32_t x = 1; x < num_boxes_axis_[0] -1; x++) {
    //         auto idx = GetBoxIndex(array<uint32_t, 3>{x, y, z});
    //         std::cout << boxes_[idx].length_ << ", ";
    //     }
    //     std::cout << std::endl;
    //   }
    //   std::cout << std::endl;
    // }
  }

  /// Calculates what the grid dimensions need to be in order to contain all the
  /// simulation objects
  ///
  /// @param      sim_objects  The simulation objects
  ///
  /// @tparam     TContainer   The container type that holds the simulation
  ///                          objects
  ///
  /// @return     The grid dimensions
  ///
  template <typename TContainer>
  void CalculateGridDimensions(const TContainer& sim_objects) {
    for (size_t i = 0; i < sim_objects.size(); i++) {
      const auto& position = sim_objects[i].GetPosition();
      auto diameter = sim_objects[i].GetDiameter();
      for (size_t j = 0; j < 3; j++) {
        if (position[j] < grid_dimensions_[2 * j]) {
          grid_dimensions_[2 * j] = position[j];
        }
        if (position[j] > grid_dimensions_[2 * j + 1]) {
          grid_dimensions_[2 * j + 1] = position[j];
        }
        if (diameter > largest_object_size_) {
          largest_object_size_ = diameter;
        }
      }
    }
  }

  /// @brief      Calculates the squared euclidian distance between two points
  ///             in 3D
  ///
  /// @param[in]  pos1  Position of the first point
  /// @param[in]  pos2  Position of the second point
  ///
  /// @return     The distance between the two points
  ///
  inline double SquaredEuclideanDistance(
      const std::array<double, 3>& pos1,
      const std::array<double, 3>& pos2) const {
    const double dx = pos2[0] - pos1[0];
    const double dy = pos2[1] - pos1[1];
    const double dz = pos2[2] - pos1[2];
    return (dx * dx + dy * dy + dz * dz);
  }

  /// @brief      Applies the given lambda to each neighbor
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  /// @param      simulation_object_id
  ///
  /// @tparam     Lambda  The type of the lambda operation
  /// @tparam     SO      The type of the simulation object
  ///
  template <typename Lambda, typename SO>
  void ForEachNeighbor(const Lambda& lambda, const SO& query,
                       size_t simulation_object_id) const {
    const auto& position = query.GetPosition();
    auto idx = GetBoxIndex(position);

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    NeighborIterator ni(neighbor_boxes);
    while (!ni.IsAtEnd()) {
      // Do something with neighbor object
      if (*ni != simulation_object_id) {
        lambda(simulation_object_id, *ni);
      }
      ++ni;
    }
  }

  // template <typename Lambda>
  // void ForEachNeighborPair(const Lambda& lambda) const {
  //   std::vector<size_t> cell_handles;
  //   for(int x = 1; x < num_boxes_axis_[0] - 1; x++) {
  //     for(int y = 1; y < num_boxes_axis_[1] - 1; y++) {
  //       auto box_idx = GetBoxIndex(array<uint32_t, 3>{x, y, 1});
  //       FixedSizeVector<const Box*, 27> boxes;
  //       GetMooreBoxes(&boxes, box_idx);
  //       // first iteration peeled off
  //       cell_handles.clear();
  //       GetCellHandles(boxes, &cell_handles);
  //       for (size_t handle : cell_handles) {
  //         lambda(handle);
  //       }
  //       for(int z = 2; z < num_boxes_axis_[2] - 1; z++) {
  //         // update boxes
  //         for(int b = 0; b < 27; b++) {
  //           ++(boxes[b]);
  //         }
  //         cell_handles.clear();
  //         GetCellHandles(boxes, &cell_handles);
  //         for (size_t handle : cell_handles) {
  //           lambda(handle);
  //         }
  //       }
  //     }
  //   }
  // }
  //
  // void GetCellHandles(const FixedSizeVector<const Box*, 27>& boxes, std::vector<size_t>* handles ) const {
  //   // determine number of cell handles
  //   size_t num_handles = 0;
  //   for (size_t i = 0; i < 27; i++) {
  //     num_handles += boxes[i]->length_;
  //   }
  //   handles->reserve(num_handles);
  //
  //   for (size_t i = 0; i < 27; i++) {
  //     auto box = boxes[i];
  //     auto current_val = box->start_;
  //     for (int j = 0; j < box->length_; j++) {
  //         handles->push_back(current_val);
  //         current_val = successors_[current_val];
  //     }
  //   }
  // }

  template <typename Lambda>
  void ForEachNeighborPair(const Lambda& lambda) const {
// #pragma omp parallel
    // {
    // size_t counter = 0;
    #pragma omp parallel for collapse(2) schedule(dynamic, 1)
    for(uint32_t z = 1; z < num_boxes_axis_[2] - 1; z++) {
    for(uint32_t y = 1; y < num_boxes_axis_[1] - 1; y++) {
        auto current_box_idx = GetBoxIndex(array<uint32_t, 3>{1, y, z});
        FixedSizeVector<size_t, 13> box_indices;
        GetMooreBoxIndices(&box_indices, current_box_idx);
        // first iteration peeled off
        // std::cout << "boxidx " << current_box_idx << " - 1 " << y << " " << z << std::endl;
        ForEachCellNeighborPair(lambda, box_indices, current_box_idx);

        for(uint32_t x = 2; x < num_boxes_axis_[0] - 1; x++) {
          // update box_indices
          ++current_box_idx;
          ++box_indices;
          // std::cout << "boxidx " << current_box_idx << " - " << x << " " << y << " " << z << std::endl;
          ForEachCellNeighborPair(lambda, box_indices, current_box_idx);
        }
      }
    }
    // #pragma omp critical
    // {
    //   counter_ += counter;
    // }
    // }
  }

  mutable size_t counter_ = 0;

  template <typename Lambda>
  void ForEachCellNeighborPair(const Lambda& lambda,
                              const FixedSizeVector<size_t, 13>& box_indices,
                              size_t current_box_idx) const {
    static int invocations = 0;
    invocations++;

    // cells in current box
    FixedSizeVector<size_t, 16> cells_current_box;
    GetCellHandles(current_box_idx, &cells_current_box);
    if (cells_current_box.size() == 0) {
      return;
    }
    // std::cout << "#cells           " << cells_current_box.size() << std::endl;
    // std::cout << "counter before   " << counter << std::endl;
    // auto before = counter;
    for (size_t n = 0; n < cells_current_box.size(); n++) {
      for (size_t c = n + 1; c < cells_current_box.size(); c++) {
          lambda(c, n);
          // lambda(n, c);
          // counter++;
          // counter++;
      }
    }
    // std::cout << "counter after 1  " << counter << " - delta: " << (counter - before) << std::endl;
    // before = counter;

    // neighbor boxes
    FixedSizeVector<size_t, 16> cells_box;
    for (size_t i = 0; i < box_indices.size(); i++) {
      size_t box_idx = box_indices[i];
      cells_box.clear();
      GetCellHandles(box_idx, &cells_box);
      for (size_t n = 0; n < cells_box.size(); n++) {
        for (size_t c = 0; c < cells_current_box.size(); c++) {
          // counter++;
          // counter++;
          lambda(cells_current_box[c], cells_box[n]);
          // lambda(cells_box[n], cells_current_box[c]);
        }
      }
      // if(cells_box.size() != 0) {
        // std::cout << "  boxidx " << box_idx << ", cells " << cells_box.size() << ", counter " << counter << " - ";
        // PrintBoxCoordinates(box_idx);
      // }
    }
    // std::cout << "counter after 2  " << counter << " - delta: " << (counter - before) << std::endl;
    // std::cout << std::endl;
  }

  void PrintBoxCoordinates(size_t idx) const {
    auto z = idx / num_boxes_xy_;
    auto remainer = idx % num_boxes_xy_;
    auto y = remainer / num_boxes_axis_[0];
    auto x = remainer % num_boxes_axis_[0];
    std::cout << x << " " << y << " " << z << std::endl;
  }

  template <typename Lambda, typename TContainer>
  void ForEachNeighborPairWithinRadius(const Lambda& lambda,
                                       const TContainer& sim_objects,
                                       double squared_radius) const {
  #pragma omp parallel
    {
    // size_t counter = 0;
    auto thread_id = omp_get_thread_num();
    #pragma omp for collapse(2) schedule(dynamic, 1)
    for(uint32_t z = 1; z < num_boxes_axis_[2] - 1; z++) {
    for(uint32_t y = 1; y < num_boxes_axis_[1] - 1; y++) {
        auto current_box_idx = GetBoxIndex(array<uint32_t, 3>{1, y, z});
        FixedSizeVector<size_t, 13> box_indices;
        GetMooreBoxIndices(&box_indices, current_box_idx);
        // first iteration peeled off
        // std::cout << "boxidx " << current_box_idx << " - 1 " << y << " " << z << std::endl;
        ForEachCellNeighborPair(thread_id, lambda, box_indices, current_box_idx, sim_objects, squared_radius);

        for(uint32_t x = 2; x < num_boxes_axis_[0] - 1; x++) {
          // update box_indices
          ++current_box_idx;
          ++box_indices;
          // std::cout << "boxidx " << current_box_idx << " - " << x << " " << y << " " << z << std::endl;
          ForEachCellNeighborPair(thread_id, lambda, box_indices, current_box_idx, sim_objects, squared_radius);
        }
      }
    }
    // #pragma omp critical
    // {
    //   counter_ += counter;
    // }
    }
  }

  template <typename Lambda, typename TContainer>
  void ForEachCellNeighborPair(size_t thread_id, const Lambda& lambda,
                              const FixedSizeVector<size_t, 13>& box_indices,
                              size_t current_box_idx,
                               const TContainer& sim_objects,
                               double squared_radius) const {
    // cells in current box
    FixedSizeVector<size_t, 16> cells_current_box;
    GetCellHandles(current_box_idx, &cells_current_box);
    if (cells_current_box.size() == 0) {
      return;
    }
    // std::cout << "#cells           " << cells_current_box.size() << std::endl;
    // std::cout << "counter before   " << counter << std::endl;
    // auto before = counter;
    // FIXME if box_length < squared_radius no distance calculations are required
    for (size_t n = 0; n < cells_current_box.size(); n++) {
      const auto& pos_n = sim_objects[cells_current_box[n]].GetPosition();
      for (size_t c = n + 1; c < cells_current_box.size(); c++) {
        const auto& pos_c = sim_objects[cells_current_box[c]].GetPosition();
        if (SquaredEuclideanDistance(pos_c, pos_n) < squared_radius) {
          lambda(thread_id, cells_current_box[c], cells_current_box[n]);
          // lambda(cells_current_box[n], cells_current_box[c]);
        }
          // counter++;
          // counter++;
      }
    }
    // std::cout << "counter after 1  " << counter << " - delta: " << (counter - before) << std::endl;
    // before = counter;

    // neighbor boxes
    FixedSizeVector<size_t, 16> cells_box;
    for (size_t i = 0; i < box_indices.size(); i++) {
      size_t box_idx = box_indices[i];
      cells_box.clear();
      GetCellHandles(box_idx, &cells_box);
      for (size_t n = 0; n < cells_box.size(); n++) {
        const auto& pos_n = sim_objects[cells_box[n]].GetPosition();
        for (size_t c = 0; c < cells_current_box.size(); c++) {
          // counter++;
          // counter++;
          const auto& pos_c = sim_objects[cells_current_box[c]].GetPosition();
          if (SquaredEuclideanDistance(pos_c, pos_n) < squared_radius) {
            lambda(thread_id, cells_current_box[c], cells_box[n]);
            // lambda(cells_box[n], cells_current_box[c]);
          }
        }
      }
      // if(cells_box.size() != 0) {
        // std::cout << "  boxidx " << box_idx << ", cells " << cells_box.size() << ", counter " << counter << " - ";
        // PrintBoxCoordinates(box_idx);
      // }
    }
    // std::cout << "counter after 2  " << counter << " - delta: " << (counter - before) << std::endl;
    // std::cout << std::endl;
  }

  void GetCellHandles(size_t box_idx, FixedSizeVector<size_t, 16>* handles ) const {
    // auto i = boxes.start_[box_idx];
    // auto current = boxes.data[i].second;
    // while (boxes.data_[i].first == box_idx) {
    //   handles->push_back(boxes.data_[i].second));
    //   i++;
    // }
    auto current = boxes_[box_idx].start_;
    for (size_t i = 0; i < boxes_[box_idx].length_; i++) {
      handles->push_back(current);
      current = successors_[current];
    }
  }

  void GetMooreBoxIndices(FixedSizeVector<size_t, 13>* neighbor_boxes,
                     size_t box_idx) const {
    // Adjacent 3 (top, left and front)
    if (adjacency_ >= kLow) {
      neighbor_boxes->push_back(box_idx + num_boxes_xy_);
      neighbor_boxes->push_back(box_idx + num_boxes_axis_[1]);
      neighbor_boxes->push_back(box_idx + 1);
    }

    // Adjacent 6
    if (adjacency_ >= kMedium) {
      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[1]);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[1]);
      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ - 1);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ + 1);
      // neighbor_boxes->push_back(box_idx - num_boxes_axis_[1] - 1);
      neighbor_boxes->push_back(box_idx + num_boxes_axis_[1] - 1);
      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[1]);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[1]);
      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ + 1);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ - 1);
      // neighbor_boxes->push_back(box_idx - num_boxes_axis_[1] + 1);
      neighbor_boxes->push_back(box_idx + num_boxes_axis_[1] + 1);
    }

    // Adjacent 4
    if (adjacency_ >= kHigh) {
      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[1] - 1);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[1] + 1);

      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[1] + 1);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[1] - 1);

      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[1] - 1);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[1] + 1);

      // neighbor_boxes->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[1] + 1);
      neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[1] - 1);
    }
  }



  /// @brief      Applies the given lambda to each neighbor or the specified
  ///             simulation object
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      sim_objects All simulation objects
  /// @param      query   The query object
  /// @param      simulation_object_id
  /// @param[in]  squared_radius  The search radius squared
  ///
  /// @tparam     Lambda      The type of the lambda operation
  /// @tparam     TContainer  The type of the simulation object container
  /// @tparam     SO          The type of the simulation object
  ///
  template <typename Lambda, typename TContainer, typename SO>
  void ForEachNeighborWithinRadius(const Lambda& lambda,
                                   const TContainer& sim_objects,
                                   const SO& query, size_t simulation_object_id,
                                   double squared_radius) {
    const auto& position = query.GetPosition();
    auto idx = GetBoxIndex(position);

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    NeighborIterator ni(neighbor_boxes);
    while (!ni.IsAtEnd()) {
      // Do something with neighbor object
      auto neighbor_index = *ni;
      if (neighbor_index != simulation_object_id) {
        const auto& neighbor_position =
            sim_objects[neighbor_index].GetPosition();
        if (SquaredEuclideanDistance(position, neighbor_position) <
            squared_radius) {
          lambda(simulation_object_id, neighbor_index);
        }
      }
      ++ni;
    }
  }

  /// @brief      Gets the size of the largest object in the grid
  ///
  /// @return     The size of the largest object
  ///
  double GetLargestObjectSize() const { return largest_object_size_; }

 private:
  /// The vector containing all the boxes in the grid
  vector<Box> boxes_;
  /// Length of a Box
  uint32_t box_length_ = 0;
  uint32_t box_length_squared_ = 0;
  /// Stores the number of boxes for each axis
  array<uint32_t, 3> num_boxes_axis_ = {{0}};
  /// Number of boxes in the xy plane (=num_boxes_axis_[0] * num_boxes_axis_[1])
  size_t num_boxes_xy_ = 0;
  /// Implements linked list - array index = key, value: next element
  vector<size_t> successors_;
  /// Determines which boxes to search neighbors in (see enum Adjacency)
  Adjacency adjacency_;
  /// The size of the largest object in the simulation
  double largest_object_size_ = 0;
  /// Cube which contains all simulation objects
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  std::array<double, 6> grid_dimensions_ = {
      {Param::kInfinity, -Param::kInfinity, Param::kInfinity, -Param::kInfinity,
       Param::kInfinity, -Param::kInfinity}};

  /// @brief      Gets the Moore (i.e adjacent) boxes of the query box
  ///
  /// @param      neighbor_boxes  The neighbor boxes
  /// @param[in]  box_idx         The query box
  ///
  void GetMooreBoxes(FixedSizeVector<const Box*, 27>* neighbor_boxes,
                     size_t box_idx) const {
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

  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  const Box* GetBoxPointer(size_t index) const { return &(boxes_[index]); }

  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  Box* GetBoxPointer(size_t index) { return &(boxes_[index]); }

  /// Returns the box index in the one dimensional array based on box
  /// coordinates in space
  ///
  /// @param      box_coord  box coordinates in space (x, y, z)
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const array<uint32_t, 3>& box_coord) const {
    return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] +
           box_coord[0];
  }

  /// @brief      Return the box index in the one dimensional array of the box
  ///             that contains the position
  ///
  /// @param[in]  position  The position of the object
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const array<double, 3>& position) const {
    array<uint32_t, 3> box_coord;
    box_coord[0] = floor(position[0] - grid_dimensions_[0]) / box_length_;
    box_coord[1] = floor(position[1] - grid_dimensions_[2]) / box_length_;
    box_coord[2] = floor(position[2] - grid_dimensions_[4]) / box_length_;

    return GetBoxIndex(box_coord);
  }
};

}  // namespace bdm

#endif  // GRID_H_
