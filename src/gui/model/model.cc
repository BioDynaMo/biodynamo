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

void Model::PrintData() {
  std::cout << "\tName: " << GetName() << '\n';
  Size_t elemCount = fModelElements.size();
  std::cout << "\tNumber of elements : " << elemCount << '\n';
  for(Int_t j = 0; j < elemCount; j++) {
    std::cout << "\tElement #" << j + 1 << '\n';
    fModelElements[j].PrintData();
  }
}

void Model::UpdateModel(std::string elementName, ModelElement& element) {}

void Model::InitializeElement(ModelElement* parent, const char* name,
                              int type) {
  ModelElement elem;
  if (type == gui::M_ENTITY_CELL) {
    if (parent == nullptr) { // top-level element
      Log::Info("Initializing top-level cell...");
    }
  } else if(type == gui::M_MODULE_GROWTH) {
    if (parent == nullptr) { // top-level element
      Log::Info("Initializing top-level module...");
    }
  } else {
    Log::Warning("Creating element of type `", type, "` not yet supported");
    return;
  }
  elem.SetElementType(type);
  elem.SetName(name);
  fModelElements.push_back(elem);
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

std::map<std::string, int> Model::GetModelElements() {
  std::map<std::string, int> elementsMap;
  for(auto i : fModelElements) {
    std::pair<std::string,int>(i.GetName(), i.GetType());
    elementsMap.insert(std::pair<std::string,int>(i.GetName(), i.GetType()));
  }
  return elementsMap;
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