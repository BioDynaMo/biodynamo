// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef BIODYNAMO_H_
#define BIODYNAMO_H_

#include <string>

#include "biology_module/grow_divide.h"
#include "biology_module/regulate_genes.h"
#include "biology_module_util.h"
#include "cell.h"
#include "compile_time_param.h"
#include "model_initializer.h"
#include "param.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "shape.h"
#include "simulation_object_util.h"
#include "variadic_template_parameter_util.h"
#include "vtune.h"
#include "bdm_imp.h"

namespace bdm {

/// This method should be called before any other biodynamo related code.
/// Parses the configuration file
/// @param executable_name sets parameter `Param::executable_name_`.
void InitializeBiodynamo(const std::string& executable_name);

/// This method should be called before any other biodynamo related code.
/// Parses command line parameters and the configuration file
/// @param argc argument count from main function
/// @param argv argument vector from main function
void InitializeBiodynamo(int argc, const char** argv);

}  // namespace bdm

#endif  // BIODYNAMO_H_
