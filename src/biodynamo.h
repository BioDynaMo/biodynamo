#ifndef BIODYNAMO_H_
#define BIODYNAMO_H_

#include "biology_module_util.h"
#include "cell.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "simulation_object_util.h"
#include "variadic_template_parameter_util.h"

namespace bdm {

#define BDM_DEFINE_BACKEND(backend) \
  struct BackendWrapper {           \
    typedef backend type;           \
  };

}  // namespace bdm

#endif  // BIODYNAMO_H_
