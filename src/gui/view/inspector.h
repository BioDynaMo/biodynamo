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

#ifndef GUI_INSPECTOR_H_
#define GUI_INSPECTOR_H_

#include "gui/controller/project.h"
#include "gui/controller/visualization_manager.h"
#include "gui/model/model_element.h"
#include "gui/view/entry.h"

#include "biodynamo.h"
#include "core/container/math_array.h"

namespace gui {

class Inspector {
 public:
  // Constructor & destructor
  Inspector(TGCompositeFrame* fMain, const char* modelName, const char* elementName);
  ~Inspector() = default;

  void UpdateAllEntries() {
    for (Entry* entry : fEntries) {
      entry->UpdateValue();
    }
    CheckSecretionBox();
  }

  Bool_t CheckSecretionBox() {
    Log::Debug("Checking secretion box!");
    Bool_t secretionChecked = (fSecretionCheckBox->GetState() == kButtonDown);
    if (secretionChecked) {
      Log::Debug("Secretion is checked!");
    } else {
      Log::Debug("Secretion is unchecked!");
    }
    SimulationEntity* entity = fModelElement->GetEntity();
    entity->SetSecretion(secretionChecked);
    std::size_t found = fElementName.find_last_of("_");
    if (found == std::string::npos) {
      Log::Error("Issue while checking secretion box! ElementName:", fElementName);
      return kFALSE;
    }
    int n = std::stoi(fElementName.substr(found + 1));
    Log::Debug("Setting red cell:", n);
    VisManager::GetInstance().SetRedCell(n, secretionChecked);
    return secretionChecked;
  }

 private:
  ModelElement*                      fModelElement;
  TGVerticalFrame*                   fV;
  TGCheckButton*                     fSecretionCheckBox;
  std::string                        fModelName;
  std::string                        fElementName;
  std::vector<TGHorizontalFrame*>    fHorizontalFrames;
  std::vector<TGNumberEntry*>        fNumericEntries;
  std::vector<TGLabel*>              fLabels;
  std::vector<Entry*>                fEntries; 
  Bool_t                             fIsInspectorValid;

  ClassDefNV(Inspector, 1);
};


}  // namespace gui

#endif // GUI_INSPECTOR_H_