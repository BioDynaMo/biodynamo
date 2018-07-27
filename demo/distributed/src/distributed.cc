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

#include <string>
#include "distributed.h"

int main(int argc, const char** argv) { return bdm::Simulate(argc, argv); }

std::string g_local_scheduler_socket_name;
std::string g_object_store_socket_name;
std::string g_object_store_manager_socket_name;
std::string g_simulation_id;

extern "C" void bdm_setup_ray(const char *local_scheduler_socket_name,
                              const char *object_store_socket_name,
                              const char *object_store_manager_socket_name,
                              const char *simulation_id) {
  g_local_scheduler_socket_name = local_scheduler_socket_name;
  g_object_store_socket_name = object_store_socket_name;
  g_object_store_manager_socket_name = object_store_manager_socket_name;
  g_simulation_id = std::string(simulation_id, 20);
}
