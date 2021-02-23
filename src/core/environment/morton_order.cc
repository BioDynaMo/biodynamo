// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/environment/morton_order.h"
#include <morton/morton.h>  // NOLINT
#include <cmath>
#include <iostream>  // FIXME remove

namespace bdm {

// -----------------------------------------------------------------------------
template <typename T>
class Stack {
 public:
  Stack(uint64_t max_elements) { data_.resize(max_elements); }

  void Push(const T& el) { data_[++top_] = el; }

  T& Top() { return data_[top_]; }
  const T& Pop() { return data_[top_--]; }
  uint64_t Size() const { return top_; }

 private:
  std::vector<T> data_;
  uint64_t top_ = 0;
};

// -----------------------------------------------------------------------------
struct Aux {
  uint64_t index = 0;
  uint64_t length = 0;
  uint64_t xpos = 0;
  uint64_t ypos = 0;
  uint64_t zpos = 0;
};

// -----------------------------------------------------------------------------
void MortonOrder::Update(const std::array<uint64_t, 3>& num_boxes_axis) {
  std::array<uint64_t, 3> max_dimensions{
      num_boxes_axis[0] - 1, num_boxes_axis[1] - 1, num_boxes_axis[2] - 1};
  auto max_dim = std::max(num_boxes_axis[0],
                          std::max(num_boxes_axis[1], num_boxes_axis[2]));
  auto max_depth =
      static_cast<uint64_t>(std::ceil(std::log(max_dim) / std::log(2)));

  offset_index_.clear();
  offset_index_.reserve(2048);

  auto length = static_cast<uint64_t>(std::pow(2, max_depth - 1));

  if (length == 0) {
    return;
  }

  Stack<Aux> s(max_depth + 1);
  s.Push({0, length, length, length, length});

  uint64_t box_cnt = 0;
  uint64_t offset = 0;
  bool record = true;

  while (s.Size() != 0) {
    auto& c = s.Top();
    // std::cout << " -----  index " << c.index << "  stack size " << s.Size()
    //           << "  length " << c.length << " xpos " << c.xpos << " ypos "
    //           << c.ypos << " zpos " << c.zpos << std::endl;
    // determine borders
    auto xmax = c.xpos;
    if (c.index == 1 || c.index == 3 || c.index == 5 || c.index == 7) {
      xmax += c.length;
    }
    auto ymax = c.ypos;
    if ((c.index > 1 && c.index < 4) || c.index > 5) {
      ymax += c.length;
    }
    auto zmax = c.zpos;
    if (c.index > 3) {
      zmax += c.length;
    }
    auto xmin = xmax - c.length;
    auto ymin = ymax - c.length;
    auto zmin = zmax - c.length;

    // std::cout << "  " << xmin << ", " << xmax << "  -  " << ymin << ", " <<
    // ymax
    //           << "  -  " << zmin << ", " << zmax << std::endl;
    if (max_dimensions[0] >= xmax - 1 && max_dimensions[1] >= ymax - 1 &&
        max_dimensions[2] >= zmax - 1) {
      // full
      record = true;
      box_cnt += c.length * c.length * c.length;
      // std::cout << "    full" << std::endl;
    } else if (max_dimensions[0] < xmin || max_dimensions[1] < ymin ||
               max_dimensions[2] < zmin) {
      // empty
      if (record) {
        // std::cout << "      record " << box_cnt << " " << offset <<
        // std::endl;
        offset_index_.push_back({box_cnt, offset});
        record = false;
      }
      auto elements = c.length * c.length * c.length;
      box_cnt += elements;
      offset += elements;
      // std::cout << "    empty " << c.length << " " << elements << std::endl;
    } else {
      auto next_length = c.length >> 1;
      // std::cout << "    " << next_length << std::endl;
      // std::cout << "    " << xmin << " , " << ymin << ", " << zmin <<
      // std::endl;
      s.Push({0, next_length, xmin + next_length, ymin + next_length,
              zmin + next_length});
      continue;
    }

    // evaluation was full or empty
    while (s.Size() > 0 && ++(s.Top().index) == 8) {
      // std::cout << "  ------------------------- POP" << std::endl;
      s.Pop();
    }
  }

  // for (auto& el : offset_index_) {
  //   std::cout << el.first << " - " << el.second << std::endl;
  // }
  // std::cout << "offset_index_ length " << offset_index_.size() << std::endl;
}

// -----------------------------------------------------------------------------
template <typename T>
std::pair<uint64_t, uint64_t> BinarySearch(uint64_t search_val,
                                           const T& container, uint64_t from,
                                           uint64_t to) {
  if (container.size() == 0) {
    return {0, 0};
  }
  if (to <= from) {
    // std::cout << "  found2 " << container[from].second << std::endl;
    return {container[from].second, from};
  }

  auto m = (from + to) / 2;
  // std::cout << " sv " << search_val << " from " << from << " to " << to << " m "
  //           << m << " val[m] " << container[m].first << std::endl;
  if (container[m].first > search_val) {
    if (m + 1 > container.size() && container[m + 1].first > search_val) {
      // std::cout << "  found " << container[m].second << std::endl;
      return {container[m].second, m};
    } else {
      return BinarySearch(search_val, container, from, m);
    }
  } else {
    return BinarySearch(search_val, container, m + 1, to);
  }
}

// -----------------------------------------------------------------------------
uint64_t MortonOrder::GetIndex(
    const std::array<uint64_t, 3>& box_coordinates, uint64_t* morton_code) const {
  auto morton = libmorton::morton3D_64_encode(
      box_coordinates[0], box_coordinates[1], box_coordinates[2]);
  // std::cout << "Morton " << morton << std::endl;
  if (morton_code != nullptr) {
    *morton_code = morton;
  }
  auto result =
      BinarySearch(morton, offset_index_, 0, offset_index_.size() - 1);
  // std::cout << result.first << " - " << result.second << std::endl;
  return morton - result.first;
}

// -----------------------------------------------------------------------------
FixedSizeVector<uint64_t, 27> MortonOrder::GetIndex(
    const FixedSizeVector<MathArray<uint64_t, 3>, 27>& box_positions) const {
  FixedSizeVector<uint64_t, 27> result;
  for (uint64_t i = 0; i < 27; ++i) {
    const auto& p = box_positions[i];
    result[i] = libmorton::morton3D_64_encode(p[0], p[1], p[2]);
  }

  std::sort(result.begin(), result.end());

  // Use result index as start value for next binary search
  // exploiting the fact that the values are sorted.
  // This reduces runtime from O(n * log(o)) to O(log(o))
  // where n is the number of box_positions and o the size of
  // the offset_index_ vector
  std::pair<uint64_t, uint64_t> partial_result = {0, 0};
  for (uint64_t i = 0; i < 27; ++i) {
    partial_result =
        BinarySearch(result[i], offset_index_, partial_result.second,
                     offset_index_.size() - 1);
    result[1] = partial_result.first;
  }
  return result;
}

}  // namespace bdm
