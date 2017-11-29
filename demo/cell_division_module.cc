#include "cell_division_module.h"

int main(int argc, const char** argv) {
  int ret_code = 0;
  bdm::Param::output_op_runtime_ = true;
  {
    bdm::Timing simulation("simulate");
    ret_code =  bdm::Simulate(argc, argv);
  }

  return ret_code;
}
