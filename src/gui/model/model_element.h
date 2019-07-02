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

#ifndef GUI_MODEL_ELEMENT_H_
#define GUI_MODEL_ELEMENT_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include <RQ_OBJECT.h>
#include <TApplication.h>
#include <TClass.h>
#include <TEnv.h>
#include <TFile.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TVirtualX.h>
#include "TObject.h"
#include "TString.h"
#include "gui/view/log.h"
#include "gui/model/simulation_entity.h"
#include "gui/model/module.h"
#include "gui/constants.h"

namespace gui {

class ModelElement {
 public:
  ModelElement() {
    if(fEntityAttributeMap.empty()) {
      /// Populate map
    }
    if(fModuleAttributeMap.empty()) {
      /// Populate map
    }
  }
  ~ModelElement() = default;
  std::string GenerateCode() {return "";};
  void        Save() {};
  Bool_t      SetElementType(int type) {
    if(fType > 0) {
      Log::Error("Element is already set for this model element!");
      return kTRUE;
    }
    if(type != M_ENTITY_CELL && type != M_MODULE_GROWTH) {
      Log::Warning("Type:", type, " not yet supported!");
      return kFALSE;
    }
    fType = type;
    return kTRUE;
  }

  void SetName(const char* name) {
    fName.assign(name);
  }

  void PrintData() {
    std::cout << "\t\tType:";
    switch(fType) {
      case M_ENTITY_CELL:
        std::cout << "Cell";
        break;
      case M_MODULE_GROWTH:
        std::cout << "Module";
        break;
      default:
        std::cout << "UNKNOWN";
    }
    std::cout << '\n';
  }

  std::string GetName() {
    return fName;
  }

  int GetType() {
    return fType;
  }

 private:
  std::string        fPathName;
  //ModelElement*      fParent;

  SimulationEntity fEntity;
  Module           fModule;
  static std::map<std::string, std::string> fEntityAttributeMap;
  static std::map<std::string, std::string> fModuleAttributeMap;
  /// TODO: add other element types

  int              fType = 0;
  std::string      fName;
  
  ClassDefNV(ModelElement,1)
};

} // namespace gui

#endif  // GUI_MODEL_ELEMENT_H_