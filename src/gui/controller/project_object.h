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

#ifndef GUI_PROJECT_OBJECT_H_
#define GUI_PROJECT_OBJECT_H_

#include <iostream>
#include <string>
#include "TObject.h"
#include "gui/constants.h"
#include "gui/view/log.h"

namespace gui {

/// This is the one and only object written into the main project file.
/// (See `Project.h`)
class ProjectObject{
 public:
  ProjectObject() {};
  ~ProjectObject() = default;
  void PrintSettings();

  /// Getter for project name
  /// @param None
  /// @return const char* private data member fProjectName
  const char* GetProjectName();

  /// Getter for test setting
  /// @param None
  /// @return const char* private data member fTestSetting
  const char* GetTestSetting();

  /// Getter for GUI version
  /// @param None
  /// @return const char* private data member fVersion
  const char* GetVersion();

  void SetProjectName(const char* name);

  ClassDef(ProjectObject, 1)

 private:
  /// Project Setting variables to be saved and loaded
  std::string fProjectName;
  std::string fTestSetting;
  std::string fVersion;
};

} // namespace gui

#endif  // GUI_PROJECT_OBJECT_H_