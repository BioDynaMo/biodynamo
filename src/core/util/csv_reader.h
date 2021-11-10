// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#ifndef CORE_UTIL_CSV_READER_H_
#define CORE_UTIL_CSV_READER_H_

#include <cmath>
#include <functional>
#include <numeric>
#include <string>
#include <vector>

#include "csv.h"

#include "core/util/io.h"

namespace bdm {

/// Reads in a CSV file using rapidcsv
class CSVReader {
 public:
  explicit CSVReader(const std::string& file, int skip = 0) : filename_(file) {
    if (!FileExists(file)) {
      std::cout << "Error: " << file << " does not exist!" << std::endl;
      exit(1);
    };
    rapidcsv::LabelParams labels(skip);
    rapidcsv::SeparatorParams separator;
    rapidcsv::ConverterParams converter(true);
    doc_ = rapidcsv::Document(file, labels, separator, converter);
    num_rows_ = GetRowCount();
    num_cols_ = GetColumnCount();
    bad_entries_.resize(num_rows_, false);
  };

  template <typename T>
  std::vector<T> GetColumn(const std::string& column_name) {
    return doc_.GetColumn<T>(column_name);
  }

  template <typename T>
  T GetCell(size_t col, size_t row) {
    return doc_.GetCell<T>(col, row);
  }

  std::string GetColumnName(size_t idx) { return doc_.GetColumnName(idx); }
  std::vector<std::string> GetColumnNames() { return doc_.GetColumnNames(); }

  size_t GetColumnCount() { return doc_.GetColumnCount(); }

  size_t GetRowCount() { return doc_.GetRowCount(); }

  size_t GetNumEntries(const std::string& column_name) {
    std::vector<std::string> ids = doc_.GetColumn<std::string>(column_name);
    return ids.size();
  }

  // Go over each valid entry of a column
  template <typename T = float, typename Lambda>
  void ForEachEntryInColumn(const std::string& column_name, Lambda lambda) {
    for (auto val : GetColumn<T>(column_name)) {
      if (!std::isnan(val)) {
        lambda(val);
      }
    }
  }

  void Print() {
    std::cout << "Filename\t\t: " << filename_ << std::endl;
    std::cout << "Row count\t\t: " << num_rows_ << std::endl;
    std::cout << "Column count\t\t: " << num_cols_ << std::endl;
    auto num_bad_entries =
        std::accumulate(bad_entries_.begin(), bad_entries_.end(), 0);
    std::cout << "Bad entries\t\t: " << num_bad_entries << std::endl;
  }

 private:
  std::string filename_;
  size_t num_rows_ = 0;
  size_t num_cols_ = 0;
  rapidcsv::Document doc_;
  // Flags if a row has a bad entry
  std::vector<bool> bad_entries_;
};

}  // namespace bdm

#endif  // CORE_UTIL_CSV_READER_H_
