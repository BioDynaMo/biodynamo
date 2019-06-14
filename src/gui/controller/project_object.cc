// Author: Lukasz Stempniewicz 25/05/19

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

#include "project_object.h"

ClassImp(ProjectObject);

using namespace gui;

void ProjectObject::PrintSettings() {
  Log::Info("Project Settings:");
  Log::Info("----------------");
  Log::Info("Project Name:");
  Log::Info("\t", fProjectName);
  Log::Info("Test Setting:");
  Log::Info("\t", fTestSetting);
  Log::Info("Version:");
  Log::Info("\t", fVersion);
  Log::Info("----------------");
}

/// Getter for project name
/// @param None
/// @return const char* private data member fProjectName
const char* ProjectObject::GetProjectName() {
  return fProjectName.c_str();
}
/// Getter for test setting
/// @param None
/// @return const char* private data member fTestSetting
const char* ProjectObject::GetTestSetting() {
  return fTestSetting.c_str();
}
/// Getter for GUI version
/// @param None
/// @return const char* private data member fVersion
const char* ProjectObject::GetVersion() {
  return fVersion.c_str();
}

void ProjectObject::SetProjectName(const char* name) {
  fProjectName.assign(name);
}

