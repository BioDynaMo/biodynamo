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

#ifndef PROGRESS_BAR_H_
#define PROGRESS_BAR_H_

#include <cstdint>
#include <iostream>
#include <string>

namespace bdm {

class ProgressBar {
 private:
  /// Time unit string for elapsed and remaining time.
  std::string time_unit_ = "s";
  /// Total number of steps to be executed.
  uint64_t total_steps_ = 0;
  /// Number of steps that have already been executed.
  uint64_t executed_steps_ = 0;
  /// Timestamp when to when the progress bar was initialized.
  int64_t start_time_ = 0;
  /// Keep track of space that is needed for printing elapsed and remaining time
  /// for nicer output.
  size_t n_terminal_chars_ = 0;
  /// Variable for conversion of time units (e.g. 1000 for ms to s).
  double time_conversion_factor_ = 1000;
  /// Boolean variable to print certain information only once in the first
  /// iteration.
  bool first_iter_ = true;
  /// Variable to detect if we write to a file or not. ProgressBar is not
  /// visible if we write to file.
  bool write_to_file_ = false;

 public:
  ProgressBar();
  explicit ProgressBar(int total_steps);

  /// Inceases the counter `executed_steps_` by `steps`.
  void Step(uint64_t steps = 1);

  /// Prints the progress bar.
  void PrintProgressBar(std::ostream& out = std::cout);

  /// Set the time unit (and the conversion factor) for elapsed and remaining
  /// time.
  void SetTimeUnit(const std::string& time_unit);
};

}  // namespace bdm

#endif  // PROGRESS_BAR_H_
