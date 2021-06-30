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

namespace bdm {

// -----------------------------------------------------------------------------
template <typename T>
class Stack {
 public:
  Stack(uint64_t max_elements) { data_.resize(max_elements); }

  void Push(const T& el) {
    data_[++top_] = el;
    assert(top_ < static_cast<int64_t>(data_.size()));
  }

  const T& Pop() {
    assert(top_ < static_cast<int64_t>(data_.size()));
    return data_[top_--];
  }

  T& Top() { return data_[top_]; }
  uint64_t Size() const { return static_cast<uint64_t>(top_ + 1); }

 private:
  std::vector<T> data_;
  int64_t top_ = -1;
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
  num_boxes_ = num_boxes_axis[0] * num_boxes_axis[1] * num_boxes_axis[2];
  std::array<uint64_t, 3> max_dimensions{
      num_boxes_axis[0] - 1, num_boxes_axis[1] - 1, num_boxes_axis[2] - 1};
  auto max_dim = std::max(num_boxes_axis[0],
                          std::max(num_boxes_axis[1], num_boxes_axis[2]));
  auto max_depth =
      static_cast<uint64_t>(std::ceil(std::log(max_dim) / std::log(2)));

  offset_index_.clear();
  offset_index_.reserve(2048);
  offset_index_.push_back({0, 0});

  if (max_depth == 0) {
    return;
  }

  auto length = static_cast<uint64_t>(std::pow(2, max_depth - 1));

  Stack<Aux> s(max_depth);
  s.Push({0, length, length, length, length});

  uint64_t box_cnt = 0;
  uint64_t offset = 0;
  bool record = false;

  while (s.Size() != 0) {
    auto& c = s.Top();
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

    if (max_dimensions[0] >= xmax - 1 && max_dimensions[1] >= ymax - 1 &&
        max_dimensions[2] >= zmax - 1) {
      // full
      if (record) {
        offset_index_.push_back({box_cnt, offset});
        record = false;
      }
      box_cnt += c.length * c.length * c.length;
    } else if (max_dimensions[0] < xmin || max_dimensions[1] < ymin ||
               max_dimensions[2] < zmin) {
      // empty
      record = true;
      auto elements = c.length * c.length * c.length;
      offset += elements;
    } else {
      auto next_length = c.length >> 1;
      s.Push({0, next_length, xmin + next_length, ymin + next_length,
              zmin + next_length});
      continue;
    }

    // evaluation was full or empty
    while (s.Size() > 0 && ++(s.Top().index) == 8) {
      s.Pop();
    }
  }
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
    if (container[from].first > search_val && from > 0) {
      from--;
    }
    return {container[from].second, from};
  }

  auto m = (from + to) / 2;
  if (container[m].first == search_val) {
    return {container[m].second, m};
  } else if (container[m].first > search_val) {
    return BinarySearch(search_val, container, from, m);
  } else {
    return BinarySearch(search_val, container, m + 1, to);
  }
}

// -----------------------------------------------------------------------------
uint64_t MortonOrder::GetMortonCode(uint64_t box_index) const {
  return BinarySearch(box_index, offset_index_, 0, offset_index_.size() - 1)
             .first +
         box_index;
}

// -----------------------------------------------------------------------------
class MortonIterator : public Iterator<uint64_t> {
 public:
  MortonIterator(uint64_t start_index, uint64_t last_index,
                 uint64_t start_morton_code, uint64_t offset_pos,
                 const std::vector<std::pair<uint64_t, uint64_t>>& offset_index)
      : next_index_(start_index),
        last_index_(last_index),
        next_morton_code_(start_morton_code),
        offset_pos_(offset_pos),
        offset_index_(offset_index) {}
  virtual ~MortonIterator() {}

  bool HasNext() const override { return next_index_ <= last_index_; }

  uint64_t Next() override {
    if (!HasNext()) {
      return 0;
    }
    auto ret = next_morton_code_;
    next_index_++;
    if (HasNext()) {
      if (offset_pos_ + 1 < offset_index_.size() &&
          offset_index_[offset_pos_ + 1].first == next_index_) {
        offset_pos_++;
      }
      next_morton_code_ = next_index_ + offset_index_[offset_pos_].second;
    }
    return ret;
  }

 private:
  uint64_t next_index_;
  uint64_t last_index_;
  uint64_t next_morton_code_ = 0;
  uint64_t offset_pos_ = 0;
  const std::vector<std::pair<uint64_t, uint64_t>>& offset_index_;
};

// -----------------------------------------------------------------------------
void MortonOrder::CallMortonIteratorConsumer(
    uint64_t start_index, uint64_t end_index,
    Functor<void, Iterator<uint64_t>*>& f) const {
  auto result =
      BinarySearch(start_index, offset_index_, 0, offset_index_.size() - 1);
  MortonIterator it(start_index, end_index, start_index + result.first,
                    result.second, offset_index_);
  f(&it);
}

}  // namespace bdm
