#ifndef CORE_MULTI_SIMULATION_RESULT_DATA_H_
#define CORE_MULTI_SIMULATION_RESULT_DATA_H_

#include <vector>

namespace bdm {

struct ResultData {
  std::vector<double> time_;
  std::vector<double> susceptible_;
  std::vector<double> infected_;
  std::vector<double> recovered_;
};

}

#endif  // CORE_MULTI_SIMULATION_RESULT_DATA_H_
