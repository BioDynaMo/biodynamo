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

#ifndef GUI_INSPECTOR_H_
#define GUI_INSPECTOR_H_

#include "gui/controller/project.h"
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
  void TestPrint() { std::cout << "Testing from inspector.\n"; }
  void UpdateAllEntries() {
    for (Entry* entry : fEntries) {
      entry->UpdateValue();
    }
  }
 private:
  ModelElement*                      fModelElement;
  TGVerticalFrame*                   fV;
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

#endif