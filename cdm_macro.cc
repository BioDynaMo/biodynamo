// gInterpreter->Load(".L /usr/lib/x86_64-linux-gnu/libomp.so");

#define BDM_SRC_DIR "/home/lukas/Documents/cern/ws/biodynamo/src"
#include "cell_division_module.h"

void cdm_macro() {
  const char* foo[1] = { "foo" };
  {
    bdm::Timing timing("simulation");
    bdm::Simulate(1, foo);
  }
}
