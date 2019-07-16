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

#include "gui/view/inspector.h"

namespace gui {

Inspector::Inspector(TGCompositeFrame* fMain, const char* modelName, const char* elementName) {
  fModelName.assign(modelName);
  fElementName.assign(fElementName);
  /// Get attributes for specific element
  fModelElement = Project::GetInstance().GetModelElement(modelName, elementName);
  
  std::map<std::string, std::string> attributeMap = fModelElement->GetAttributeMap();
  std::map<std::string, std::string>::iterator it;
  fV = new TGVerticalFrame(fMain, 200, 300);
  TGLayoutHints *fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);
  for(it = attributeMap.begin(); it!=attributeMap.end(); ++it) {
    Log::Debug("Inspector: Will create attribute frame for");
    Log::Debug("  Attribute:", it->first, ", type:", it->second);
    Entry* entry = new Entry(fV, this, it->first.c_str(), it->second.c_str());
    entry->Init(fModelElement);
    fEntries.push_back(entry);
    if(it->first.find("Position") != std::string::npos || it->first.find("TractorForce") != std::string::npos) {
      fV->AddFrame(entry, fL1);
    }
  }
  /// Ensure that Position and TractorForce are added first
  for(Entry *entry : fEntries) {
    std::string entryName(entry->GetEntryName());
    if(entryName.find("Position") == std::string::npos && 
       entryName.find("TractorForce") == std::string::npos) 
    {
      fV->AddFrame(entry, fL1);
    }
  }
  
  fMain->AddFrame(fV, fL1);

  for(Entry* entry : fEntries) {
    entry->EnableEventHandling();
  }
}

} // namespace gui