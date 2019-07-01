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

#include "gui/model/model.h"
#include <TROOT.h>
#include "gui/constants.h"
#include "gui/view/log.h"
#include "TList.h"
#include "gui/model/model_element.h"
#include "gui/model/simulation_entity.h"

namespace gui {

void Model::CreateModel() {}

void Model::SaveModel() {}

void Model::SimulateModel() {}

void Model::SetName(const char* name) {
  fModelName.assign(name);
}

const char* Model::GetName() {
  return fModelName.c_str();
}

Int_t Model::GetElementCount() {
  return fEntities.size() + fModules.size();
}

void Model::PrintData() {
  std::cout << "\tName: " << GetName() << '\n';
  Size_t elemCount = fEntities.size();
  std::cout << "\tNumber of elements (simulation entities): " << elemCount << '\n';
  for(Int_t j = 0; j < elemCount; j++) {
    std::cout << "\tSimulation Entity #" << j + 1 << '\n';
    fEntities[j].PrintData();
  }
  elemCount = fModules.size();
  std::cout << "\tNumber of elements (modules): " << elemCount << '\n';
  for(Int_t j = 0; j < elemCount; j++) {
    std::cout << "\tModule #" << j + 1 << '\n';
    fModules[j].PrintData();
  }
}

void Model::UpdateModel(std::string elementName, ModelElement& element) {}

void Model::InitializeElement(ModelElement* parent, const char* name,
                              int type) {
  if (type == gui::M_ENTITY_CELL) {
    SimulationEntity elem;
    elem.SetName(name);
    if (parent == nullptr) { // top-level element
      Log::Info("Initializing cell...");
      fEntities.push_back(elem);
    }
  } else if(type == gui::M_MODULE_GROWTH) {
    Module module;
    module.SetName(name);
    if (parent == nullptr) { // top-level element
      Log::Info("Initializing module...");
      fModules.push_back(module);
    }
  }
}

Bool_t Model::CreateElement(const char* parent, const char* name, int type) {
  if (strcmp(parent, "") == 0) {
    // Signifies top level element
    ModelElement* tmp = FindElement(name);
    if (tmp != nullptr) {
      gui::Log::Error("Cannot create element! Already exists: ");
      return kFALSE;
    } else {
      InitializeElement(nullptr, name, type);
    }
  } else {
    ModelElement* tmp = FindElement(parent);
    if (tmp != nullptr) {
      gui::Log::Error("Cannot create element! Parent does not exist");
      return kFALSE;
    }
  }
  return kTRUE;
}

std::vector<std::string> Model::GetSimulationEntities() {
  std::vector<std::string> names;
  for(auto i : fEntities) {
    names.push_back(std::string(i.GetName()));
  }
  return names;
}

std::vector<std::string> Model::GetModules() {
  std::vector<std::string> names;
  for(auto i : fModules) {
    names.push_back(std::string(i.GetName()));
  }
  return names;
}

ModelElement* Model::FindElement(const char* elementName) {
  //TObjLink* lnk = fModelElements->FirstLink();
  //while (lnk) {
  //  ModelElement* tmp = (ModelElement*)lnk->GetObject();
  //  tmp = tmp->SearchChildren(elementName);
  //  if (tmp != nullptr)
  //    return tmp;
  //  lnk = lnk->Next();
  //}
  return nullptr;
}

std::string Model::GenerateCode() {
  std::string code("");
  return code;
}

} // namespace gui