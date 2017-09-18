#ifndef GRID_H_
#define GRID_H_

#include <omp.h>
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include "fixed_size_vector.h"
#include "inline_vector.h"
#include "param.h"
// #include "resource_manager.h"
#include "cell.h"
#include "simulation_object_vector.h"

namespace bdm {

using std::array;
using std::vector;
using std::fmod;

// TODO move and document
template <typename T, uint64_t N>
class CircularBuffer {
public:
  CircularBuffer() {
    for (uint64_t i = 0; i < N; i++) {
      data_[i] = T();
    }
  }

  void clear() {
    position_ = 0;
    for (uint64_t i = 0; i < N; i++) {
      data_[i].clear();
    }
  }

  void push_back(const T& data) {
    data_[position_] = data;
    // position_ %= N;
    Increment();
  }

  T& operator[](uint64_t idx) {
    return data_[(idx+position_) % N];
  }

  const T& operator[](uint64_t idx) const {
    return data_[(idx+position_) % N];
  }

  T* End() {
    return &(data_[position_]);
  }

  void Increment() {
    position_++;
    position_ %= N;
  }

private:
  T data_[N];
  uint64_t position_ = 0;
};

/// A class that represents Cartesian 3D grid
template <typename TResourceManager = ResourceManager<>>
class Grid {
 public:
  /// A single unit cube of the grid
  struct Box {
    /// start value of the linked list of simulatio objects inside this box.
    /// Next element can be found at `successors_[start_]`
    std::atomic<SoHandle> start_;
    /// length of the linked list
    std::atomic<uint16_t> length_;

    Box() : start_(SoHandle()), length_(0) {}
    /// Copy Constructor required for boxes_.resize()
    /// Since box values will be overwritten afterwards it forwards to the
    /// default ctor
    Box(const Box& other) : Box() {}
    /// Required for boxes_.resize
    /// Since box values will be overwritten afterwards, implementation is
    /// missing
    const Box& operator=(const Box& other) const { return *this; }

    bool IsEmpty() const { return length_ == 0; }

    /// @brief      Adds a simulation object to this box
    ///
    /// @param[in]  obj_id  The object's identifier
    ///
    template <typename TSimulationObjectVector>
    void AddObject(SoHandle obj_id, TSimulationObjectVector* successors) {
      length_++;
      auto old_start = std::atomic_exchange(&start_, obj_id);
      if (old_start != SoHandle()) {
        (*successors)[obj_id] = old_start;
      }
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
        if (countdown_ > 0) {
          current_value_ = grid_->successors_[current_value_];
        }
        return *this;
      }

      const SoHandle& operator*() const { return current_value_; }

      Grid<TResourceManager>* grid_;
      SoHandle current_value_;
      int countdown_;
    };

    template <typename TGrid = Grid<TResourceManager>>
    Iterator begin() const {  // NOLINT
      return Iterator(&(TGrid::GetInstance()), this);
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

    const SoHandle& operator*() const { return *box_iterator_; }

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
    typename Box::Iterator box_iterator_;
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
  /// @param[in]  adjacency    The adjacency (see #Adjacency)
  void Initialize(Adjacency adjacency = kHigh) {
    adjacency_ = adjacency;

    UpdateGrid();
  }

  virtual ~Grid() {}

  /// Gets the singleton instance
  static Grid<TResourceManager>& GetInstance() {
    static Grid<TResourceManager> kGrid;
    return kGrid;
  }

  /// Clears the grid
  void ClearGrid() {
    boxes_.clear();
    box_length_ = 1;
    largest_object_size_ = 0;
    num_boxes_axis_ = {{0}};
    num_boxes_xy_ = 0;
    int32_t inf = std::numeric_limits<int32_t>::max();
    grid_dimensions_ = {inf, -inf, inf, -inf, inf, -inf};
    successors_.clear();
  }

  /// Updates the grid, as simulation objects may have moved, added or deleted
  void UpdateGrid() {
    ClearGrid();

    auto inf = Param::kInfinity;
    array<double, 6> tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
    CalculateGridDimensions(&tmp_dim);
    np_grid_dimensions_ = tmp_dim;
    RoundOffGridDimensions(tmp_dim);

    auto los = ceil(largest_object_size_);
    box_length_ = (los == 0) ? 1 : los;

    for (int i = 0; i < 3; i++) {
      int dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      int r = dimension_length % box_length_;
      // If the grid is not perfectly divisible along each dimension by the
      // resolution, extend the grid so that it is
      if (r != 0) {
        // std::abs for the case that box_length_ > dimension_length
        grid_dimensions_[2 * i + 1] += (box_length_ - r);
      } else {
        // Else extend the grid dimension with one row, because the outmost
        // object lies exactly on the border
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

      // num_boxes_axis_[i] = dimension_length / box_length_ + 1;
    }

    num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];
    auto total_num_boxes = num_boxes_xy_ * num_boxes_axis_[2];

    if (boxes_.size() != total_num_boxes) {
      boxes_.resize(total_num_boxes, Box());
    }

    successors_.Initialize();

    // Assign simulation objects to boxes
    auto rm = TResourceManager::Get();
    rm->ApplyOnAllElementsParallel([this](auto&& sim_object, SoHandle id) {
      const auto& position = sim_object.GetPosition();
      auto idx = this->GetBoxIndex(position);
      auto box = this->GetBoxPointer(idx);
      box->AddObject(id, &successors_);
      sim_object.SetBoxIdx(idx);
    });
  }

  /// Calculates what the grid dimensions need to be in order to contain all the
  /// simulation objects
  void CalculateGridDimensions(array<double, 6>* ret_grid_dimensions) {
    auto rm = TResourceManager::Get();

    const auto max_threads = omp_get_max_threads();

    std::vector<std::array<double, 6>*> all_grid_dimensions(max_threads,
                                                            nullptr);
    std::vector<double*> all_largest_object_size(max_threads, nullptr);

#pragma omp parallel
    {
      auto thread_id = omp_get_thread_num();
      auto* grid_dimensions = new std::array<double, 6>;
      *grid_dimensions = {{Param::kInfinity, -Param::kInfinity,
                           Param::kInfinity, -Param::kInfinity,
                           Param::kInfinity, -Param::kInfinity}};
      double* largest_object_size = new double;
      *largest_object_size = 0;
      all_grid_dimensions[thread_id] = grid_dimensions;
      all_largest_object_size[thread_id] = largest_object_size;

      rm->ApplyOnAllTypes([&](auto* sim_objects, uint16_t type_idx) {
#pragma omp for
        for (size_t i = 0; i < sim_objects->size(); i++) {
          const auto& position = (*sim_objects)[i].GetPosition();
          for (size_t j = 0; j < 3; j++) {
            if (position[j] < (*grid_dimensions)[2 * j]) {
              (*grid_dimensions)[2 * j] = position[j];
            }
            if (position[j] > (*grid_dimensions)[2 * j + 1]) {
              (*grid_dimensions)[2 * j + 1] = position[j];
            }
          }
          auto diameter = (*sim_objects)[i].GetDiameter();
          if (diameter > *largest_object_size) {
            *largest_object_size = diameter;
          }
        }
      });

#pragma omp master
      {
        for (int i = 0; i < max_threads; i++) {
          for (size_t j = 0; j < 3; j++) {
            if ((*all_grid_dimensions[i])[2 * j] <
                (*ret_grid_dimensions)[2 * j]) {
              (*ret_grid_dimensions)[2 * j] = (*all_grid_dimensions[i])[2 * j];
            }
            if ((*all_grid_dimensions[i])[2 * j + 1] >
                (*ret_grid_dimensions)[2 * j + 1]) {
              (*ret_grid_dimensions)[2 * j + 1] =
                  (*all_grid_dimensions[i])[2 * j + 1];
            }
          }
          if ((*all_largest_object_size[i]) > largest_object_size_) {
            largest_object_size_ = *(all_largest_object_size[i]);
          }
        }
      }
    }

    for (auto element : all_grid_dimensions) {
      delete element;
    }
    for (auto element : all_largest_object_size) {
      delete element;
    }
  }

  void RoundOffGridDimensions(const array<double, 6>& grid_dimensions) {
    grid_dimensions_[0] = floor(grid_dimensions[0]);
    grid_dimensions_[2] = floor(grid_dimensions[2]);
    grid_dimensions_[4] = floor(grid_dimensions[4]);
    grid_dimensions_[1] = ceil(grid_dimensions[1]);
    grid_dimensions_[3] = ceil(grid_dimensions[3]);
    grid_dimensions_[5] = ceil(grid_dimensions[5]);
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
                       const SoHandle& simulation_object_id) const {
    const auto& position = query.GetPosition();
    auto idx = GetBoxIndex(position);

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    NeighborIterator ni(neighbor_boxes);
    while (!ni.IsAtEnd()) {
      // Do something with neighbor object
      if (*ni != simulation_object_id) {
        lambda(*ni);
      }
      ++ni;
    }
  }

  /// @brief      Applies the given lambda to each neighbor or the specified
  ///             simulation object
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  /// @param      simulation_object_id
  /// @param[in]  squared_radius  The search radius squared
  ///
  /// @tparam     Lambda      The type of the lambda operation
  /// @tparam     SO          The type of the simulation object
  ///
  template <typename Lambda, typename SO>
  void ForEachNeighborWithinRadius(const Lambda& lambda, const SO& query,
                                   const SoHandle& simulation_object_id,
                                   double squared_radius) {
    const auto& position = query.GetPosition();
    auto idx = query.GetBoxIdx();

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    NeighborIterator ni(neighbor_boxes);
    while (!ni.IsAtEnd()) {
      // Do something with neighbor object
      SoHandle neighbor_handle = *ni;
      if (neighbor_handle != simulation_object_id) {
        auto rm = TResourceManager::Get();
        rm->ApplyOnElement(neighbor_handle, [&](auto&& sim_object) {
          const auto& neighbor_position = sim_object.GetPosition();
          if (this->SquaredEuclideanDistance(position, neighbor_position) <
              squared_radius) {
            lambda(sim_object, neighbor_handle);
          }
        });
      }
      ++ni;
    }
  }

  // TODO
  template <typename Lambda>
  void ForEachNeighborPairWithinRadius(const Lambda& lambda,
                                       double squared_radius) const {
    uint32_t z_start, y_start;
    auto rm = TResourceManager::Get();
    auto cells = rm->template Get<Cell>();
    for (uint16_t i = 0; i < 9; i++) {
      switch (i) {
        case 0:
          z_start = 1;
          y_start = 1;
          break;
        case 1:
          z_start = 1;
          y_start = 2;
          break;
        case 2:
          z_start = 1;
          y_start = 3;
          break;
        case 3:
          z_start = 2;
          y_start = 1;
          break;
        case 4:
          z_start = 2;
          y_start = 2;
          break;
        case 5:
          z_start = 2;
          y_start = 3;
          break;
        case 6:
          z_start = 3;
          y_start = 1;
          break;
        case 7:
          z_start = 3;
          y_start = 2;
          break;
        case 8:
          z_start = 3;
          y_start = 3;
          break;
      }

#pragma omp parallel for collapse(2) \
    schedule(dynamic, 1) firstprivate(z_start, y_start)
      for (uint32_t z = z_start; z < num_boxes_axis_[2] - 1; z += 3) {
        for (uint32_t y = y_start; y < num_boxes_axis_[1] - 1; y += 3) {
          auto current_box_idx = GetBoxIndex(array<uint32_t, 3>{1, y, z});
          FixedSizeVector<size_t, 14> box_indices;
          GetHalfMooreBoxIndices(&box_indices, current_box_idx);
          // get all cell handles
          CircularBuffer<FixedSizeVector<SoHandle, 16>, 14> so_handles; // TODO change to InlineVector
          GetAllCellHandles(box_indices, &so_handles);

          // first iteration peeled off
          ForEachCellNeighborPair(lambda, cells, so_handles, 0, squared_radius);

          for (uint32_t x = 2; x < num_boxes_axis_[0] - 1; x++) {
            // update box_indices
            ++box_indices;
            // so_handles.clear();
            // GetAllCellHandles(box_indices, &so_handles);
            // ForEachCellNeighborPair(lambda, cells, so_handles, 0, squared_radius);
            UpdateCellHandles(box_indices, &so_handles);
            ForEachCellNeighborPair(lambda, cells, so_handles, 4, squared_radius);

            // CircularBuffer<FixedSizeVector<SoHandle, 16>, 14> ref; // TODO change to InlineVector
            // GetAllCellHandles(box_indices, &ref);

          }
        }
      }
    }
  }

  void GetAllCellHandles(const FixedSizeVector<size_t, 14>& box_indices,
                         CircularBuffer<FixedSizeVector<SoHandle, 16>, 14>* cell_handles) const {
    for (uint64_t i = 0; i < box_indices.size(); i++) {
      // FixedSizeVector<SoHandle, 16> handles;
      // GetCellHandles(box_indices[i], &handles);
      // cell_handles->push_back(handles);
      GetCellHandles(box_indices[i], cell_handles->End());
      cell_handles->Increment();
    }
  }

  void UpdateCellHandles(const FixedSizeVector<size_t, 14>& box_indices,
                         CircularBuffer<FixedSizeVector<SoHandle, 16>, 14>* cell_handles) const {
    for (uint64_t i = 9; i < 14; i++) {
      // FixedSizeVector<SoHandle, 16> handles;
      // GetCellHandles(box_indices[i], &handles);
      // cell_handles->push_back(handles);
      auto handles = cell_handles->End();
      handles->clear();
      GetCellHandles(box_indices[i], handles);
      cell_handles->Increment();
    }
    // swap cell handles from C and BW
    // const auto& tmp = (*cell_handles)[0];
    // (*cell_handles)[0] = (*cell_handles)[4];
    // (*cell_handles)[4] = tmp;
  }

  // TODO
  // template <typename Lambda>
  template <typename Lambda, typename TSimObj>
  void ForEachCellNeighborPair(const Lambda& lambda,
                               TSimObj* cells,
                              //  TResourceManager* rm,
                               const CircularBuffer<FixedSizeVector<SoHandle, 16>, 14> so_handles,
                               uint64_t current_box_idx,
                               double squared_radius) const {
    const auto& cells_current_box = so_handles[current_box_idx];
    if (cells_current_box.size() == 0) {
      return;
    }
    if (cells_current_box.size() > 1) {
      // FIXME if box_length < squared_radius no distance calculations are
      // required
      for (size_t n = 0; n < cells_current_box.size(); n++) {
        // rm->ApplyOnElement(cells_current_box[n], [&,this](auto&& element_n){
          auto&& element_n = (*cells)[cells_current_box[n].GetElementIdx()];

          const auto& pos_n = element_n.GetPosition();
          for (size_t c = n + 1; c < cells_current_box.size(); c++) {
            // rm->ApplyOnElement(cells_current_box[c], [&,this](auto&& element_c){
              auto&& element_c = (*cells)[cells_current_box[c].GetElementIdx()];

              const std::array<double, 3>& pos_c = element_c.GetPosition();
              if (this->SquaredEuclideanDistance(pos_c, pos_n) < squared_radius) {
                lambda(element_c, cells_current_box[c], element_n, cells_current_box[n]);
              }
            // });

          }
        // });

      }
    }

    // neighbor boxes
    for (size_t i = 0; i < 14; i++) {
      if (i == current_box_idx) continue;
      const auto& cells_box = so_handles[i];
      for (size_t n = 0; n < cells_box.size(); n++) {
        // rm->ApplyOnElement(cells_box[n], [&,this](auto&& element_n){
          auto&& element_n = (*cells)[cells_box[n].GetElementIdx()];

          const auto& pos_n = element_n.GetPosition();
          for (size_t c = 0; c < cells_current_box.size(); c++) {
            // rm->ApplyOnElement(cells_current_box[c], [&,this](auto&& element_c){
              auto&& element_c = (*cells)[cells_current_box[c].GetElementIdx()];

              const auto& pos_c = element_c.GetPosition();
              if (this->SquaredEuclideanDistance(pos_c, pos_n) < squared_radius) {
                lambda(element_c, cells_current_box[c], element_n, cells_box[n]);
              }
            // });
          }
        // });
      }
    }
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
    box_coord[0] = (floor(position[0]) - grid_dimensions_[0]) / box_length_;
    box_coord[1] = (floor(position[1]) - grid_dimensions_[2]) / box_length_;
    box_coord[2] = (floor(position[2]) - grid_dimensions_[4]) / box_length_;

    return GetBoxIndex(box_coord);
  }

  /// Gets the size of the largest object in the grid
  double GetLargestObjectSize() const { return largest_object_size_; }

  array<int32_t, 6>& GetGridDimensions() { return grid_dimensions_; }

  std::array<uint64_t, 3> GetBoxCoordinates(size_t box_idx) const {
    std::array<uint64_t, 3> box_coord;
    box_coord[2] = box_idx / num_boxes_xy_;
    auto remainder = box_idx % num_boxes_xy_;
    box_coord[1] = remainder / num_boxes_axis_[0]; // TODO correct index?
    box_coord[0] = remainder % num_boxes_axis_[0];
    return box_coord;
  }

 private:
  /// The vector containing all the boxes in the grid
  vector<Box> boxes_;
  /// Length of a Box
  uint32_t box_length_ = 1;
  /// Stores the number of boxes for each axis
  array<uint32_t, 3> num_boxes_axis_ = {{0}};
  /// Number of boxes in the xy plane (=num_boxes_axis_[0] * num_boxes_axis_[1])
  size_t num_boxes_xy_ = 0;
  /// Implements linked list - array index = key, value: next element
  ///
  ///     // Usage
  ///     SoHandle current_element = ...;
  ///     SoHandle next_element = successors_[current_element];
  SimulationObjectVector<SoHandle, TResourceManager> successors_;
  /// Determines which boxes to search neighbors in (see enum Adjacency)
  Adjacency adjacency_;
  /// The size of the largest object in the simulation
  double largest_object_size_ = 0;
  /// Cube which contains all simulation objects
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  std::array<int32_t, 6> grid_dimensions_;
  std::array<double, 6> np_grid_dimensions_;

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
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + 1));
    }

    // Adjacent 12
    if (adjacency_ >= kMedium) {
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_axis_[0] + 1));
    }

    // Adjacent 8
    if (adjacency_ >= kHigh) {
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1));
    }
  }
public:  // TODO remove
  void Print(uint64_t box_idx) const {
    static int i = 0;
    auto coord = GetBoxCoordinates(box_idx);
    std::cout << i++ << (i <= 9 ? "  " : " ") << ((int)coord[0]) - 1 << " " << ((int)coord[1]) - 2 << " " << ((int)coord[2]) - 3 << std::endl;
  }
  // TODO
  void GetHalfMooreBoxIndices(FixedSizeVector<size_t, 14>* neighbor_boxes,
                     size_t box_idx) const {
     // C
     neighbor_boxes->push_back(box_idx);

     // BW
     neighbor_boxes->push_back(box_idx + num_boxes_axis_[0] - 1);
    //  Print(box_idx + num_boxes_axis_[0] - 1);

     // FNW
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1);
    //  Print(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1);

     // NW
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ -1);
    //  Print(box_idx + num_boxes_xy_ -1);

     // BNW
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1);
    //  Print(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1);

     // B
     neighbor_boxes->push_back(box_idx + num_boxes_axis_[0]);
    //  Print(box_idx + num_boxes_axis_[0]);

     // FN
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0]);
    //  Print(box_idx + num_boxes_xy_ - num_boxes_axis_[0]);

     // N
     neighbor_boxes->push_back(box_idx + num_boxes_xy_);
    //  Print(box_idx + num_boxes_xy_);

     // BN
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0]);
    //  Print(box_idx + num_boxes_xy_ + num_boxes_axis_[0]);

     // E
     neighbor_boxes->push_back(box_idx + 1);
    //  Print(box_idx + 1);

     // BE
     neighbor_boxes->push_back(box_idx + num_boxes_axis_[0] + 1);
    //  Print(box_idx + num_boxes_axis_[0] + 1);

     // FNE
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1);
    //  Print(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1);

     // NE
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ + 1);
    //  Print(box_idx + num_boxes_xy_ + 1);

     // BNE
     neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1);
    //  Print(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1);
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

  // TODO
  void GetCellHandles(size_t box_idx, FixedSizeVector<SoHandle, 16>* handles ) const {
    // SoHandle current = static_cast<SoHandle>(boxes_[box_idx].start_);
    auto size = boxes_[box_idx].length_.load(std::memory_order_relaxed);
    if(size == 0) {
      return;
    }
    auto current = boxes_[box_idx].start_.load(std::memory_order_relaxed);
    for (size_t i = 0; i < size - 1u; i++) {
      handles->push_back(current);
      current = successors_[current];
    }
    handles->push_back(current);
  }
};

}  // namespace bdm

#endif  // GRID_H_
