#include "param.h"

namespace bdm {

// every 30 min by default
uint32_t Param::backup_every_x_seconds_ = 1800;

bool Param::live_visualization_ = false;

bool Param::export_visualization_ = false;

uint32_t Param::write_freq_ = 1;

bool Param::bound_space_ = false;

double Param::lbound_ = 0;

double Param::rbound_ = 100;

bool Param::run_physics_ = true;

bool Param::display_timers_ = false;

}  // namespace bdm
