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

#include "core/util/progress_bar.h"
#include <unistd.h>
#include <cassert>
#include <iomanip>
#include <string>
#include "core/util/timing.h"

namespace bdm {

ProgressBar::ProgressBar() : ProgressBar(0){};

ProgressBar::ProgressBar(int total_steps)
    : total_steps_(total_steps),
      executed_steps_(0),
      start_time_(Timing::Timestamp()),
      first_iter_(true),
      n_digits_time_(0),
      write_to_file_(false) {}

void ProgressBar::Step(uint64_t steps) { executed_steps_ += steps; }

void ProgressBar::PrintProgressBar(std::ostream &out) {
  // Do not write to file because it does not handle "\r" well.
  if (write_to_file_) {
    return;
  }

  // Print empty line at first iteration to compensate for "\r" below.
  if (first_iter_) {
    // Determine if we write to file or terminal.
    if (!isatty(fileno(stdout))) {
      std::cout << "<ProgressBar::PrintProgressBar> omits output because you "
                   "write to file."
                << std::endl;
      write_to_file_ = true;
      return;
    }
    out << "ET = Elapsed Time; TR = Time Remaining" << std::endl;
    first_iter_ = false;
  }

  // If total_steps_ is not set, we fall back to simple step printing. This can
  // be the case if someone uses SimulateUntil where we do not know the number
  // of steps in advance.
  if (total_steps_ == 0) {
    out << "Time step: " << total_steps_ << std::endl;
  }

  // 1. Get current and compute elapsed time
  int64_t current_time = Timing::Timestamp();
  real_t elapsed_time = static_cast<real_t>(current_time - start_time_);

  // 2. Compute ETA
  real_t fraction_computed =
      static_cast<real_t>(executed_steps_) / static_cast<real_t>(total_steps_);
  assert(fraction_computed <= 1);
  real_t remaining_time = elapsed_time / fraction_computed - elapsed_time;

  // 3. Print progress bar
  size_t n_steps_computed = std::floor(fraction_computed / 0.02);
  int n_digits_steps = std::ceil(std::log10(total_steps_));
  n_digits_time_ =
      std::max({static_cast<int>(std::ceil(std::log10(elapsed_time))),
                static_cast<int>(std::ceil(std::log10(remaining_time))),
                n_digits_time_});

  // Progress bar
  out << "[" << std::string(n_steps_computed, '#')
      << std::string(50 - n_steps_computed, ' ') << "] ";
  // Print number of executed steps
  out << std::setw(n_digits_steps) << executed_steps_ << " / "
      << std::setw(n_digits_steps) << total_steps_ << " ";
  // Print remaining time
  out << "( ET: " << std::setw(n_digits_time_) << elapsed_time
      << ", TR:" << std::setw(n_digits_time_) << std::ceil(remaining_time)
      << ")[ms]";
  // Override lines in next step
  out << "\r";

  // 3. Go to new line once we've reached the last step
  if (executed_steps_ == total_steps_) {
    out << "\n";
  }
}

}  // namespace bdm
