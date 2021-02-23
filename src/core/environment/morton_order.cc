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
    std::cout << " -----  index " << c.index << "  stack size " << s.Size()
              << "  length " << c.length << " xpos " << c.xpos << " ypos "
              << c.ypos << " zpos " << c.zpos << std::endl;
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

    std::cout << "  " << xmin << ", " << xmax << "  -  " << ymin << ", " << ymax
              << "  -  " << zmin << ", " << zmax << std::endl;
    if (max_dimensions[0] >= xmax - 1 && max_dimensions[1] >= ymax - 1 &&
        max_dimensions[2] >= zmax - 1) {
      // full
      record = true;
      box_cnt += c.length * c.length * c.length;
      std::cout << "    full" << std::endl;
    } else if (max_dimensions[0] < xmin || max_dimensions[1] < ymin ||
               max_dimensions[2] < zmin) {
      // empty
      if (record) {
        std::cout << "      record " << box_cnt << " " << offset << std::endl;
        offset_index_.push_back({box_cnt, offset});
        record = false;
      }
      auto elements = c.length * c.length * c.length;
      box_cnt += elements;
      offset += elements;
      std::cout << "    empty " << c.length << " " << elements << std::endl;
    } else {
      auto next_length = c.length >> 1;
      std::cout << "    " << next_length << std::endl;
      std::cout << "    " << xmin << " , " << ymin << ", " << zmin << std::endl;
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

  for (auto& el : offset_index_) {
    std::cout << el.first << " - " << el.second << std::endl;
  }
  std::cout << "offset_index_ length " << offset_index_.size() << std::endl;
}

// -----------------------------------------------------------------------------
uint64_t MortonOrder::GetIndex(
    const std::array<uint64_t, 3>& box_coordinates) const {}

// -----------------------------------------------------------------------------
FixedSizeVector<uint64_t, 27> MortonOrder::GetIndex(
    const FixedSizeVector<std::array<uint64_t, 3>, 27>& boxes) const {}

}  // namespace bdm
