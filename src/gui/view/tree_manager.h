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

#ifndef GUI_TREE_MANAGER_H_
#define GUI_TREE_MANAGER_H_

#include <TGListTree.h>
#include <TROOT.h>
#include <TString.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

#include "gui/controller/project.h"
#include "gui/view/log.h"

namespace gui {

struct ModelTree {
  std::string fName;
  TGListTreeItem* fBaseItem;
  TGListTreeItem* fElementsBaseItem;
  std::vector<TGListTreeItem*> fElementsListTreeItems;
  TGListTreeItem* fSimulationSettingsListTreeItem;
  std::vector<std::string> fModelElementNames;
};

class TreeManager {
 public:
  TreeManager() {
    isProjectCreated = kFALSE;

    std::unordered_map<std::string, TString> fileMapping;

    TString iconname(gProgPath);
#ifdef R__WIN32
    iconname += "\\icons\\";
#else
    iconname += "/icons/";
#endif

    fileMapping["Project"] = iconname + "tree_project.xpm";
    fileMapping["Model"] = iconname + "tree_model.xpm";
    fileMapping["SimulationSetup"] = iconname + "settings.xpm";
    fileMapping["Cell"] = iconname + "cell.png";
    fileMapping["GrowthModule"] = iconname + "growth.png";

    const TGPicture* pic = 0;

    for (auto it = fileMapping.begin(); it != fileMapping.end(); ++it) {
      TString fileName(it->second);
      pic = gClient->GetPicture(fileName);
      fIconMap[it->first] = pic;
    }
  }
  ~TreeManager() {
    if(isProjectCreated) {
      fProjectListTree->DeleteItem(gProjectListTreeItem);
      fProjectListTree->Cleanup();
    }
  };

  const TGPicture* GetIcon(std::string iconName) {
    std::unordered_map<std::string, const TGPicture*>::const_iterator got =
        fIconMap.find(iconName);
    if (got != fIconMap.end()) {
      return got->second;
    }
    return 0;  // Intentionally zero
  }

  const char* CreateTopLevelElement(int type, std::string name = "") {
    std::string elementName("Other");
    const TGPicture* pic = 0;
    Bool_t isNameValid;
    if (type == M_ENTITY_CELL) {
        elementName.assign("Cell");
        pic = GetIcon("Cell");
    } else if (type == M_MODULE_GROWTH) {
      elementName.assign("Growth");
      pic = GetIcon("GrowthModule");
    }
  
    if(name.empty()) {
      elementName.append("_0");
      isNameValid = kFALSE;
    } else {
      elementName.assign(name);
      isNameValid = kTRUE;
    }
    
    Int_t i;
    while (!isNameValid) {
      std::size_t found = elementName.find_last_of("_");
      if (found == std::string::npos) {
        Log::Error("Cannot name new element!");
        return "";
      }
      int n = std::stoi(elementName.substr(found + 1));
      std::string str = std::to_string(n + 1);
      Log::Debug("Changing elementName from:", elementName);
      elementName.replace(found + 1, std::string::npos, str);
      Log::Debug("to:", elementName);
      isNameValid = IsElementNameAvailable(elementName.c_str());
    }
    if((i = GetModelIndex(fCurModelName.c_str())) > -1) {
      fCurListItem = fModelTrees[i].fElementsBaseItem;
      TGListTreeItem* LTItem = fProjectListTree->AddItem(fCurListItem, elementName.c_str(), pic, pic);
      fModelTrees[i].fElementsListTreeItems.emplace_back(LTItem);
      fModelTrees[i].fModelElementNames.emplace_back(elementName);
      fProjectListTree->OpenItem(fCurListItem);
    }
    return elementName.c_str();
  }

  void CreateProjectTree(TGListTree* projectTree, std::string& projectName) {
    if (!isProjectCreated) {
      isProjectCreated = kTRUE;
      fProjectListTree = projectTree;
      fCurListItem = 0;
      TGListTreeItem* projectLTItem =
          fProjectListTree->AddItem(fCurListItem, projectName.c_str(),
                                    GetIcon("Project"), GetIcon("Project"));
      fCurListItem = projectLTItem;
      gProjectListTreeItem = projectLTItem;
      fCurModelName = "";
    }
  }

  void CreateModelTree(Model &model) {
    Log::Info("Will create model tree from pre-existing model!");
    std::string modelName(model.GetName());
    CreateModelTree(modelName);

    std::map<std::string, int> elementsMap = model.GetModelElements();
    std::map<std::string, int>::iterator it;

    for (it = elementsMap.begin(); it!=elementsMap.end(); ++it) {
      CreateTopLevelElement(it->second, it->first);
    }
  }

  void CreateModelTree(std::string& modelName) {
    if (isProjectCreated) {
      if(GetModelIndex(modelName.c_str()) > -1) {
        Log::Warning("Model '", modelName, "' already exists! Cannot create!");
        return;
      }
      Log::Info("Creating Model Tree");
      // name is available
      ModelTree tmpTree;
      tmpTree.fName = modelName;
      tmpTree.fBaseItem =
          fProjectListTree->AddItem(gProjectListTreeItem, modelName.c_str(),
                                    GetIcon("Model"), GetIcon("Model"));
      tmpTree.fElementsBaseItem =
          fProjectListTree->AddItem(tmpTree.fBaseItem, "Model Elements");
      tmpTree.fSimulationSettingsListTreeItem = fProjectListTree->AddItem(
          tmpTree.fBaseItem, "Settings", GetIcon("SimulationSetup"),
          GetIcon("SimulationSetup"));
      fModelTrees.emplace_back(tmpTree);
      fProjectListTree->OpenItem(gProjectListTreeItem);
      fProjectListTree->SetSelected(tmpTree.fElementsBaseItem);
      fProjectListTree->OpenItem(tmpTree.fBaseItem);
      fCurModelName = modelName;
    }
  }

  Int_t GetModelIndex(const char* modelName) {
    Int_t modelCount = fModelTrees.size();
    for(Int_t i = 0; i < modelCount; i++) {
      if(fModelTrees[i].fName.compare(modelName) == 0) {
        return i;
      }
    }
    return -1;
  }

  std::string IsModelSelected() {
    TGListTreeItem* selectedItem = fProjectListTree->GetSelected();
    const char* itemName = selectedItem->GetText();
    std::string tmpName = "";
    Log::Info("Selected tree item: ", itemName);
    for (u_int i = 0; i < fModelTrees.size(); i++) {
      if (fModelTrees[i].fName.compare(itemName) == 0)
        tmpName = fModelTrees[i].fName;
      if (fProjectListTree->FindChildByName(selectedItem, itemName))
        tmpName = fModelTrees[i].fName;
      if (selectedItem == fModelTrees[i].fElementsBaseItem ||
          selectedItem == fModelTrees[i].fSimulationSettingsListTreeItem)
        tmpName = fModelTrees[i].fName;
      if (std::find(fModelTrees[i].fModelElementNames.begin(),
                    fModelTrees[i].fModelElementNames.end(),
                    itemName) != fModelTrees[i].fModelElementNames.end())
        tmpName = fModelTrees[i].fName;

      if (!tmpName.empty())
        break;
    }
    fCurModelName.assign(tmpName);
    return tmpName;
  }

  std::string GetCurrentSelectedModelName() { return fCurModelName; }
  TGListTreeItem* gProjectListTreeItem;  // base TGListTree item

 private:
  Bool_t IsElementNameAvailable(const char* elementName) {
    for (ModelTree tree : fModelTrees)
      for (std::string str : tree.fModelElementNames)
        if (str.compare(elementName) == 0)
          return kFALSE;
    return kTRUE;
  }
  Bool_t isProjectCreated = kFALSE;
  TGListTreeItem* fCurListItem;  // current TGListTreeItem (level) in TGListTree

  std::vector<ModelTree> fModelTrees;
  std::string fCurModelName;
  TGListTree* fProjectListTree;  // base TGListTree

  std::unordered_map<std::string, const TGPicture*> fIconMap;
};

}  // namespace gui

#endif  // GUI_TREE_MANAGER_H_