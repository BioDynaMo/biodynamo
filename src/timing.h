#ifndef TIMING_H_
#define TIMING_H_

#include <chrono>
#include <iostream>
#include <string>

#include "param.h"
#include "timing_aggregator.h"

namespace bdm {

class Timing {
 public:
  typedef std::chrono::high_resolution_clock Clock;

  explicit Timing(const std::string& description = "")
      : start_{Timestamp()}, text_{description} {}

  Timing(const std::string& description, TimingAggregator* aggregator)
      : start_{Timestamp()}, text_{description}, aggregator_{aggregator} {}

  ~Timing() {
    int64_t duration = (Timestamp() - start_);
    if (aggregator_ == nullptr) {
      std::cout << text_ << " " << duration << " ms" << std::endl;
    } else {
      aggregator_->AddEntry(text_, duration);
    }
  }

  int64_t Timestamp() {
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    auto time = Clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = duration_cast<milliseconds>(since_epoch);
    return millis.count();
  }

 private:
  int64_t start_;
  std::string text_;
  TimingAggregator* aggregator_ = nullptr;
};

}  // namespace bdm

#endif  // TIMING_H_
