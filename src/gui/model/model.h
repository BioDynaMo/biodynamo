// Author: Lukasz Stempniewicz 21/08/19

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

#ifndef GUI_MODEL_H_
#define GUI_MODEL_H_

#include <iostream>
#include <string>
#include <vector>

#include <TROOT.h>
#include <TClass.h>
#include <TFile.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <RQ_OBJECT.h>
#include "TObject.h"

#include "gui/constants.h"
#include "gui/model/model_element.h"
#include "gui/model/simulation_entity.h"
#include "gui/model/module.h"
#include "gui/view/log.h"

enum SimulationState { kIDLE ,kSIMULATING, kDONE };

namespace gui {

class Model {
 public:
  /// Constructor and destructor
  Model() {};
 ~Model() = default;

  Bool_t        fModified;

  void          SetName(const char* name);
  const char*   GetName();
  void          CreateModel();
  void          PrintData();
  void          SaveModel();
  void          SimulateModel();
  void          UpdateModel(std::string elementName, ModelElement& element);
  void          IsElementNameAvailable();
  Bool_t        CreateElement(const char* parent, const char* name, int type, bdm::Double3 pos = {0, 0, 0});
  void          GenerateCode(Bool_t diffusion);
  std::string   GetModelFolder(Bool_t createFolder=kFALSE);
  std::string   GetBackupFile();

  void          EnableGridPos() {fUseGridPos = kTRUE;}

  std::map<std::string, int> GetModelElements();
  ModelElement*              GetModelElement(const char* name);
 
 private:
  std::string                   fModelName;
  std::vector<ModelElement>     fModelElements;
  std::string                   fSimulationBackupFilename = "backup.root";
  Bool_t                        fUseGridPos = kFALSE;
  Bool_t        CreateDirectory(const char* dirPath);
  void          InitializeElement(ModelElement* parent, const char* name, int type, bdm::Double3 pos = {0, 0, 0});
  ModelElement* FindElement(const char* elementName);
  void          UpdateLastCellPosition(ModelElement* elem, bdm::Double3 presetPos = {0, 0, 0});

  ClassDefNV(Model,1)
};

}  // namespace gui

#endif // GUI_MODEL_H_