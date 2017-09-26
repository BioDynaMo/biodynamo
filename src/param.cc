#include "param.h"

namespace bdm {

// every 30 min by default
uint32_t Param::backup_every_x_seconds_ = 1800;

bool Param::use_paraview_ = false;

bool Param::write_to_file_ = false;

std::size_t Param::write_freq_ = 1;

}  // namespace bdm
