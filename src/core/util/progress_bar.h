#ifndef PROGRESS_BAR_H_
#define PROGRESS_BAR_H_

#include <cstdint>
#include <iostream>

namespace bdm {

class ProgressBar {
 private:
  /// Total number of steps to be executed.
  uint64_t total_steps_;
  /// Number of steps that have already been executed.
  uint64_t executed_steps_;
  /// Timestamp when to when the progress bar was initialized.
  int64_t start_time_;
  /// Boolean variable to print certain informatoin only once in the first
  /// iteration.
  bool first_iter_;
  /// Keep track of space that is needed for printing elapsed and remaining time
  /// for nicer output.
  int n_digits_time_;

 public:
  ProgressBar();
  ProgressBar(int total_steps);

  /// Inceases the counter `executed_steps_` by `steps`.
  void Step(uint64_t steps = 1);

  /// Prints the progress bar.
  void PrintProgressBar(std::ostream &out = std::cout);
};

}  // namespace bdm

#endif  // PROGRESS_BAR_H_