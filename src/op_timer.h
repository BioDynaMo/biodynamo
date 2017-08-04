#ifndef OP_TIMER_H_
#define OP_TIMER_H_

#include <string>
#include "timing.h"

namespace bdm {

using std::string;

/// \brief Decorator for `Operations` to measure runtime
template <typename TOp>
struct OpTimer {
  explicit OpTimer(string timer_msg) : timer_msg_(timer_msg) {}
  explicit OpTimer(string timer_msg, const TOp& op)
      : timer_msg_(timer_msg), operation_(op) {}

  template <typename Container>
  void operator()(Container* cells) {
    {
      Timing timer(timer_msg_);
      operation_(cells);
    }
  }

 private:
  string timer_msg_;
  TOp operation_;
};

}  // namespace bdm

#endif  // OP_TIMER_H_
