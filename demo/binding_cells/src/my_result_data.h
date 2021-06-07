#ifndef MY_RESULT_DATA_H_
#define MY_RESULT_DATA_H_

#include <vector>

#include "TH2I.h"

#include "core/multi_simulation/result_data.h"

namespace bdm {

struct MyResultData : public ResultData {
  // The results of interest
  std::vector<double> activity;
  double initial_concentration;
  TH2I activation_intensity;

  std::vector<double> activity;

  double operator(ResultData* o) override {
    MyResultData other = static_cast<MyResultData>(o);
    return compute(this->activity, other->activity);
  }
};

}  // namespace bdm

#endif  // MY_RESULT_DATA_H_
