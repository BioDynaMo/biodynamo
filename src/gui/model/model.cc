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

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "gui/model/model.h"
#include "gui/controller/project.h"
#include "gui/controller/generate_template.h"

namespace gui {

std::map<std::string, std::string> ModelElement::fEntityAttributeMap;
std::map<std::string, std::string> ModelElement::fModuleAttributeMap;

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
  size_t elemCount = fModelElements.size();
  std::cout << "\tNumber of elements : " << elemCount << '\n';
  for (uint32_t j = 0; j < elemCount; j++) {
    std::cout << "\tElement #" << j + 1 << '\n';
    fModelElements[j].PrintData();
  }
}

void Model::UpdateModel(std::string elementName, ModelElement& element) {}

void Model::InitializeElement(ModelElement* parent, const char* name,
                              int type, bdm::Double3 pos) {
  ModelElement elem;
  if (type == gui::M_ENTITY_CELL) {
    if (parent == nullptr) { // top-level element
      Log::Info("Initializing top-level cell...");
    }
  } else if (type == gui::M_MODULE_GROWTH) {
    if (parent == nullptr) { // top-level element
      Log::Info("Initializing top-level module...");
    }
  } else {
    Log::Warning("Creating element of type `", type, "` not yet supported");
    return;
  }
  fModelElements.push_back(elem);
  size_t elemIdx = fModelElements.size() - 1;
  fModelElements[elemIdx].SetElementType(type);
  fModelElements[elemIdx].SetName(name);
  ModelElement* elemPtr = &(fModelElements[elemIdx]);
  if (type == gui::M_ENTITY_CELL) {
    UpdateLastCellPosition(elemPtr, pos);
  }
}

void Model::UpdateLastCellPosition(ModelElement* elem, bdm::Double3 presetPos) {
  if (!fUseGridPos) {
    std::map<std::string, int> elementsMap = GetModelElements();
    std::map<std::string, int>::iterator it;
  
    uint32_t cellCount = 0;
  
    for (it = elementsMap.begin(); it!=elementsMap.end(); ++it) {
      if (it->second == M_ENTITY_CELL) {
        cellCount++;
      }
    }
    Log::Debug("cellCount in `UpdateLastCellPosition`:", cellCount);
    SimulationEntity* entity = elem->GetEntity();
    Int_t spacing = 50;
    double zPos = ((cellCount - 1) / 4) * spacing;
    double yPos, xPos;
    if      (cellCount % 4 == 0) { xPos = 50; yPos = 50; }
    else if (cellCount % 4 == 1) { xPos = 0;  yPos = 0;  } 
    else if (cellCount % 4 == 2) { xPos = 0;  yPos = 50; } 
    else if (cellCount % 4 == 3) { xPos = 50; yPos = 0;  }
    else {
      Log::Error("Should never reach this point!! (in `UpdateLastCellPosition`)");
    }
    bdm::Double3 pos({xPos, yPos, zPos});
    entity->SetPosition(pos);
  }
  else {
    fUseGridPos = kFALSE;
    SimulationEntity* entity = elem->GetEntity();
    entity->SetPosition(presetPos);
  }
}

Bool_t Model::CreateElement(const char* parent, const char* name, int type, bdm::Double3 pos) {
  if (strcmp(parent, "") == 0) {
    // Signifies top level element
    ModelElement* tmp = FindElement(name);
    if (tmp != nullptr) {
      gui::Log::Error("Cannot create element! Already exists: ");
      return kFALSE;
    } else {
      InitializeElement(nullptr, name, type, pos);
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
  for (auto i : fModelElements) {
    std::pair<std::string,int>(i.GetName(), i.GetType());
    elementsMap.insert(elementsMap.end(), std::pair<std::string,int>(i.GetName(), i.GetType()));
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

ModelElement* Model::GetModelElement(const char* name) {
  std::string tmpName;
  size_t elemCount = fModelElements.size();
  for (uint32_t i = 0; i < elemCount; i++) {
    tmpName.assign(fModelElements[i].GetName());
    if (tmpName.compare(name) == 0) {
      Log::Debug("Found element:", name);
      return &(fModelElements[i]);
    }
  }
  return nullptr;
}

Bool_t Model::CreateDirectory(const char* dirPath) {
  int check; 
  check = mkdir(dirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
  if (!check) {
    return kTRUE;
  }
  return kFALSE;
}

std::string Model::GetModelFolder(Bool_t createFolder) {
  std::string modelFolderPath(Project::GetInstance().GetProjectPath());
  modelFolderPath.append(fModelName);

  if (createFolder) {
    struct stat info;
  
    if (stat(modelFolderPath.c_str(), &info) != 0) {
      Log::Debug("Cannot access ", modelFolderPath );
      Log::Debug("Attempting to create directory...");
      if (CreateDirectory(modelFolderPath.c_str())) {
        Log::Debug("Directory created!");
      } else {
        Log::Error("Could not create directory!");
      }
    }
    else if (info.st_mode & S_IFDIR) {
      Log::Debug(modelFolderPath, " is a directory");
      std::string cmd = bdm::Concat("rm -rf ", modelFolderPath);
      Int_t retVal = system(cmd.c_str());
      Log::Debug("`", cmd, "` returned:", retVal);
      if (retVal == 0) {
        return GetModelFolder(createFolder);
      } else {
        Log::Error("Could not delete:`", modelFolderPath, "`");
      }
    }
    else
      Log::Debug(modelFolderPath, " is not a directory!");
  }
  return modelFolderPath;
}

/// Will retreive the backup file relative to the model folder
std::string Model::GetBackupFile() {
  std::string backupFile = bdm::Concat(GetModelFolder(), "/build/guibackup.root");
  return backupFile;
}

/// Generates source code for this model
void Model::GenerateCode(Bool_t diffusion) {
  std::string folderPath = bdm::Concat(GetModelFolder(kTRUE), "/");
  std::string srcPath = bdm::Concat(folderPath, "src/");
  std::string srcFileH = bdm::Concat(srcPath, fModelName, ".h");
  std::string srcFileCC = bdm::Concat(srcPath, fModelName, ".cc");
  std::string cMakeFile = bdm::Concat(folderPath, "CMakeLists.txt");

  Bool_t diffusionEnabled = diffusion;
  std::string BioModulesName("");
  std::string srcFileBioModuleH("");
  GenerateTemplate::GetInstance().EnableDiffusion(diffusionEnabled);

  if (diffusionEnabled) {
    BioModulesName.assign("diffusion_modules.h");
    srcFileBioModuleH = srcPath + BioModulesName;
  }

  std::string cMakeOutput = 
    GenerateTemplate::GetInstance().GenerateCMakeLists(fModelName.c_str());
  ofstream myfile;
  myfile.open(cMakeFile.c_str());
  myfile << cMakeOutput;
  myfile.close();

  if (CreateDirectory(srcPath.c_str())) {
    std::string srcCC = 
      GenerateTemplate::GetInstance().GenerateSrcCC(fModelName.c_str());
    myfile.open(srcFileCC.c_str());
    myfile << srcCC;
    myfile.close();
  
    std::string srcH = 
      GenerateTemplate::GetInstance().GenerateSrcH(fModelName.c_str(), fSimulationBackupFilename.c_str(), BioModulesName.c_str());
    myfile.open(srcFileH.c_str());
    myfile << srcH;
    myfile.close();

    if (diffusionEnabled) {
      std::string srcBioModule = 
        GenerateTemplate::GetInstance().GenerateDiffusionModuleSrcH();
      myfile.open(srcFileBioModuleH.c_str());
      myfile << srcBioModule;
      myfile.close();
    }
  } else {
    Log::Error("Could not create folder:`", srcPath, "`");
  }
}

}  // namespace gui