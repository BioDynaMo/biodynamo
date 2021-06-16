
#ifndef CORE_MULTI_SIMULATION_DATABASE_H_
#define CORE_MULTI_SIMULATION_DATABASE_H_

#include "core/analysis/time_series.h"

namespace bdm {

using experimental::TimeSeries;

class Database {
 public:
  TimeSeries data_;

  static Database* GetInstance() {
    static Database kDatabase;
    return &kDatabase;
  }

 private:
  Database() {}
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_DATABASE_H_