#ifndef GRID_H_
#define GRID_H_

#include <iostream> // TODO
#include <array>
#include <cmath>
#include <vector>
#include <limits>
#include "inline_vector.h"
#include "matrix.h"

namespace bdm {

using std::array;
using std::vector;
using std::fmod;

class Grid {
public:
  struct Box {
    // uint64_t start_ = 0;
    // uint16_t length = 0;
    size_t cell_id_ = std::numeric_limits<size_t>::max();

    bool IsEmpty() const {
      return cell_id_ == std::numeric_limits<size_t>::max();
    }

    void AddCell(size_t cell_id) {
      if (IsEmpty()) {
        cell_id_ = cell_id;
      } else {
        throw "foo";
      }
    }
  };

  struct NeighborIterator {
    NeighborIterator(InlineVector<const Box*, 27>* neighbor_boxes) : neighbor_boxes_(neighbor_boxes) {}

    bool IsEnd() const { return is_end_; }

    size_t operator*() const {
      return (*neighbor_boxes_)[box_idx_]->cell_id_;
    }

    /// version where empty neighbors in neighbor_boxes_ are allowed
    NeighborIterator& operator++() {
      while (++box_idx_ < neighbor_boxes_->size()) {
        if ((*neighbor_boxes_)[box_idx_]->IsEmpty()) {
          continue;
        }
        return *this;
      }
      is_end_ = true;
      return *this;
    }

    /// version with no empty neighbors in neighbor_boxes_
    // NeighborIterator& operator++() {
    //   box_idx_++;
    //   if (box_idx_ == neighbor_boxes_->size()) {
    //     is_end_ = true;
    //   }
    //   return *this;
    // }

  private:
    InlineVector<const Box*, 27>* neighbor_boxes_;
    int box_idx_ = 0;
    bool is_end_ = false;
  };

  Grid(vector<array<double, 3>>& positions, uint32_t box_length, const std::array<double, 3> max_value) : positions_(positions), box_lenght_(box_length) {
    // FIXME for simplicity assumes no negative positions
    num_boxes_axis_[0] = static_cast<size_t>(max_value[0]) / box_length;
    num_boxes_axis_[1] = static_cast<size_t>(max_value[1]) / box_length;
    num_boxes_axis_[2] = static_cast<size_t>(max_value[2]) / box_length;

    num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];
    auto total_num_boxes = num_boxes_xy_ * num_boxes_axis_[2];

    std::cout << "num_boxes x " << num_boxes_axis_[0] << std::endl;
    std::cout << "num_boxes y " << num_boxes_axis_[1] << std::endl;
    std::cout << "num_boxes z " << num_boxes_axis_[2] << std::endl;
    std::cout << "num_boxes xy " << num_boxes_xy_ << std::endl;

    boxes_.resize(total_num_boxes);

    // assign simulation objects to boxes
    for (size_t cell_id = 0; cell_id < positions_.size(); cell_id++) {
      auto& position = positions_[cell_id];
      auto box = GetBoxPointer(GetBoxIndex(position));
      box->AddCell(cell_id);
    }
  }

  void ForEachNeighbor() {
    vector<size_t> sum(positions_.size());
#pragma omp parallel for
    for(size_t i = 0; i < positions_.size(); i++) {
      auto& position = positions_[i];
      auto idx = GetBoxIndex(position);

      InlineVector<const Box*, 27> neighbor_boxes;
      GetMooreBoxes(&neighbor_boxes, idx);
      GetMooreBoxes(&neighbor_boxes, idx);
      GetMooreBoxes(&neighbor_boxes, idx);

      NeighborIterator it(&neighbor_boxes);
      while(!it.IsEnd()) {
        // volatile auto current_val = *it;

        // do something with it
        sum[i] = *it;

        ++it;
      }
    }
    std::cout << "cell id sum " << sum[4] << std::endl;
  }

private:
  vector<Box> boxes_;
  /// length of a Box
  uint32_t box_lenght_;
  /// stores the number of boxes for each axis
  array<uint32_t, 3> num_boxes_axis_;
  /// number of boxes in the xy plane (=num_boxes_axis_[0] * num_boxes_axis_[1])
  size_t num_boxes_xy_;
  vector<array<double, 3>>& positions_;

  void GetMooreBoxes(InlineVector<const Box*, 27>* neighbor_boxes, size_t box_idx) {
    neighbor_boxes->push_back(GetBoxPointer(box_idx));
    auto box_coord = GetBoxCoordinates(box_idx);

    if (box_coord[0] > 0) {
      auto coord = Add(box_coord, {-1, 0, 0});
      auto box = GetBoxPointer(GetBoxIndex(coord));
      // if (!box->IsEmpty())
        neighbor_boxes->push_back(box);
    }

    if (box_coord[0] < num_boxes_axis_[0] - 1) {
      auto coord = Add(box_coord, {1, 0, 0});
      auto box = GetBoxPointer(GetBoxIndex(coord));
      // if (!box->IsEmpty())
        neighbor_boxes->push_back(box);
    }

    if (box_coord[1] > 0) {
      auto coord = Add(box_coord, {0, -1, 0});
      auto box = GetBoxPointer(GetBoxIndex(coord));
      // if (!box->IsEmpty())
        neighbor_boxes->push_back(box);
    }

    if (box_coord[1] < num_boxes_axis_[1] - 1) {
      auto coord = Add(box_coord, {0, 1, 0});
      auto box = GetBoxPointer(GetBoxIndex(coord));
      // if (!box->IsEmpty())
        neighbor_boxes->push_back(box);
    }

    if (box_coord[2] > 0) {
      auto coord = Add(box_coord, {0, 0, -1});
      auto box = GetBoxPointer(GetBoxIndex(coord));
      // if (!box->IsEmpty())
        neighbor_boxes->push_back(box);
    }

    if (box_coord[2] < num_boxes_axis_[2] - 1) {
      auto coord = Add(box_coord, {0, 0, 1});
      auto box = GetBoxPointer(GetBoxIndex(coord));
      // if (!box->IsEmpty())
        neighbor_boxes->push_back(box);
    }

  }

  const Box* GetBoxPointer(size_t index) const {
    return &(boxes_[index]);
  }

  Box* GetBoxPointer(size_t index) {
    return &(boxes_[index]);
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
    return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] + box_coord[0];
   }

  /// Return the box index in the one dimensional array of the box that contains
  /// the position
  size_t GetBoxIndex(const array<double, 3> position) {
    array<uint32_t, 3> box_coord;
    box_coord[0] = floor(position[0]) / box_lenght_;
    box_coord[1] = floor(position[1]) / box_lenght_;
    box_coord[2] = floor(position[2]) / box_lenght_;

    // std::cout << "GetBoxIndex " << position[0] << " - " << position[1] << " - " << position[2] << " || "
    //           << box_coord[0] << " - " << box_coord[1] << " - " << box_coord[2] << std::endl;
    return GetBoxIndex(box_coord);
  }

  // TODO move to matrix
  array<uint32_t, 3> Add(const array<uint32_t, 3>& a, const array<int, 3>&& b) {
    array<uint32_t, 3> ret;
    ret[0] = a[0] + b[0];
    ret[1] = a[1] + b[1];
    ret[2] = a[2] + b[2];
    return ret;
  }
};

}   // namespace bdm

#endif  // GRID_H_
