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

inline double TimeConversionFactor(const std::string& unit) {
  if (unit == "ms") {
    return 1;
  } else if (unit == "s") {
    return 1000;
  } else if (unit == "min") {
    return 1000 * 60;
  } else {
    return 1000 * 60 * 60;
  }
}

ProgressBar::ProgressBar() : ProgressBar(0){};

ProgressBar::ProgressBar(int total_steps)
    : total_steps_(total_steps), start_time_(Timing::Timestamp()) {}

void ProgressBar::Step(uint64_t steps) { executed_steps_ += steps; }

void ProgressBar::PrintProgressBar(std::ostream& out) {
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
  auto elapsed_time = static_cast<real_t>(current_time - start_time_);

  // 2. Compute ETA
  real_t fraction_computed =
      static_cast<real_t>(executed_steps_) / static_cast<real_t>(total_steps_);
  assert(fraction_computed <= 1);
  real_t remaining_time = elapsed_time / fraction_computed - elapsed_time;
  elapsed_time /= time_conversion_factor_;
  remaining_time /= time_conversion_factor_;

  // 3. Print progress bar
  size_t n_steps_computed = std::floor(fraction_computed / 0.02);

  // Write terminal ouput to string first to determine the number of digits
  std::string terminal_output = "";
  terminal_output.append("[")
      .append(std::string(n_steps_computed, '#'))
      .append(std::string(50 - n_steps_computed, ' '))
      .append("] ");
  terminal_output.append(std::to_string(executed_steps_))
      .append(" / ")
      .append(std::to_string(total_steps_))
      .append(" ");
  terminal_output.append("( ET: ")
      .append(std::to_string(elapsed_time))
      .append(", TR:")
      .append(std::to_string(remaining_time))
      .append(")[")
      .append(time_unit_)
      .append("]");
  terminal_output.append("\r");

  // Update number of terminal characters
  n_terminal_chars_ = std::max({terminal_output.length(), n_terminal_chars_});

  // If the string is shorter than n_terminal_chars_, we need to add spaces
  // to override the previous output
  if (terminal_output.length() < n_terminal_chars_) {
    terminal_output.append(
        std::string(n_terminal_chars_ - terminal_output.length(), ' '));
  }

  // Print to terminal
  out << terminal_output;

  // 3. Go to new line once we've reached the last step
  if (executed_steps_ == total_steps_) {
    out << "\n";
  }
}

void ProgressBar::SetTimeUnit(const std::string& time_unit) {
  // Accept only the following time units: ms, s, min, h
  if (time_unit == "ms" || time_unit == "s" || time_unit == "min" ||
      time_unit == "h") {
    time_unit_ = time_unit;
    time_conversion_factor_ = TimeConversionFactor(time_unit);
  } else {
    Log::Warning("ProgressBar::SetTimeUnit",
                 "Time unit not supported. Use ms, s, min, h instead. ",
                 "Falling back to [s].");
  }
}

}  // namespace bdm
