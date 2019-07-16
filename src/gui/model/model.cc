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

ModelElement* Model::GetModelElement(const char* name) {
  std::string tmpName;
  Size_t elemCount = fModelElements.size();
  for(Int_t i = 0; i < elemCount; i++) {
    tmpName.assign(fModelElements[i].GetName());
    if(tmpName.compare(name) == 0) {
      Log::Debug("Found element:", name);
      return &(fModelElements[i]);
    }
  }
  return nullptr;
}


Bool_t Model::CreateDirectory(const char* dirPath) {
  int check; 
  check = mkdir(dirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
  if(!check) {
    return kTRUE;
  }
  return kFALSE;
}

std::string Model::GetModelFolder(Bool_t createFolder) {
  std::string modelFolderPath(Project::GetInstance().GetProjectPath());
  modelFolderPath.append(fModelName);
  struct stat info;

  if( stat( modelFolderPath.c_str(), &info ) != 0 ) {
    Log::Debug("Cannot access ", modelFolderPath );
    Log::Debug("Attempting to create directory...");
    if(createFolder) {
      if(CreateDirectory(modelFolderPath.c_str())) {
        Log::Debug("Directory created!");
      } else {
        Log::Error("Could not create directory!");
      }
    } 
  }
  else if( info.st_mode & S_IFDIR ) {
    Log::Debug(modelFolderPath, " is a directory");
    std::string cmd = "rm -rf " + modelFolderPath;
    if(system(cmd.c_str())) {
      return GetModelFolder(createFolder);
    } else {
      Log::Error("Could not delete:`", modelFolderPath, "`");
    }
  }
  else
      Log::Debug(modelFolderPath, " is not a directory!");
  
  return modelFolderPath;
}

void Model::GenerateCode() {
  std::string folderPath = GetModelFolder(kTRUE) + "/";
  std::string srcPath = folderPath + "src/";
  std::string srcFileH = srcPath + fModelName + ".h";
  std::string srcFileCC = srcPath + fModelName + ".cc";
  std::string cMakeFile = folderPath + "CMakeLists.txt";

  std::string cMakeOutput = 
    GenerateTemplate::GetInstance().GenerateCMakeLists(fModelName.c_str());
  ofstream myfile;
  myfile.open(cMakeFile.c_str());
  myfile << cMakeOutput;
  myfile.close();

  if(CreateDirectory(srcPath.c_str())) {
    std::string srcCC = 
      GenerateTemplate::GetInstance().GenerateSrcCC(fModelName.c_str());
    myfile.open(srcFileCC.c_str());
    myfile << srcCC;
    myfile.close();
  
    std::string srcH = 
      GenerateTemplate::GetInstance().GenerateSrcH(fModelName.c_str());
    myfile.open(srcFileH.c_str());
    myfile << srcH;
    myfile.close();
  } else {
    Log::Error("Could not create folder:`", srcPath, "`");
  }
}


} // namespace gui