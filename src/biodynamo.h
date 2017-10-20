#ifndef BIODYNAMO_H_
#define BIODYNAMO_H_

#include "biology_module_util.h"
#include "cell.h"
#include "compile_time_param.h"
#include "model_initializer.h"
#include "param.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "simulation_object_util.h"
#include "variadic_template_parameter_util.h"
#include "vtune.h"

namespace bdm {

void InitializeBioDynamo(int argc, const char** argv);

}  // namespace bdm

#endif  // BIODYNAMO_H_
