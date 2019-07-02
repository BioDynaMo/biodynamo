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
#include "gui/model/model.h"

namespace gui {

/// This is the one and only object written into the main project file.
/// (See `Project.h`)
class ProjectObject{
 public:
  ProjectObject() {};
  ~ProjectObject() = default;

  void PrintData() {
    std::cout << "\nPrinting data for Project: " << fProjectName << '\n';
    std::cout << "TestSetting: " << fTestSetting << '\n';
    std::cout << "Version: " << fVersion << '\n';
    Size_t modelCount = fModels.size();
    std::cout << "Number of models: " << modelCount << '\n';
    for(Int_t i = 0; i < modelCount; i++) {
      fModels[i].PrintData();
    }
  }

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

  void CreateModel(const char* name) {
    Model newModel;
    newModel.SetName(name);
    fModels.push_back(newModel);
    PrintData();
  }

  std::vector<Model>* GetModels() {
    return &fModels;
  }

  Model* GetModel(const char* modelName) {
    Int_t modelCount = fModels.size();
    for(Int_t i = 0; i < modelCount; i++) {
      if(strcmp(fModels[i].GetName(), modelName) == 0) {
        Log::Debug("ProjectObject::GetModel found model:", modelName);
        return &fModels[i];
      }
    }
    return nullptr;
  }

  void Clear(){
    fProjectName.clear();
    fTestSetting.clear();
    fVersion.clear();
    fModels.clear();
  }

  void TestInit() {
    fVersion.assign("1.0");
    fTestSetting.assign("SomeTestSetting");
  }

  void SetProjectName(const char* name);

  ClassDefNV(ProjectObject, 1)

 private:
  /// Project Setting variables to be saved and loaded
  std::string fProjectName;
  std::string fTestSetting;
  std::string fVersion;
  std::vector<Model> fModels; // Purposefully not a vector of pointers
};

} // namespace gui

#endif  // GUI_PROJECT_OBJECT_H_