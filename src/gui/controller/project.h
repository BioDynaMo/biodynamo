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

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// This File contains the declaration of the Project-class              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef GUI_PROJECT_H_
#define GUI_PROJECT_H_

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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

#include "core/util/io.h"
#include "gui/model/model.h"
#include "gui/view/log.h"
#include "gui/controller/project_object.h"

namespace gui {

enum class SimulationType { kBuild, kRun };

/// Project represents the intermediate communication mechanism
/// between the GUI and its models. `gui.h` will contain only 1 instance
/// of Project and will fetch as well as update the model through this instance.
/// This design will allow the GUI to be more-or-less independent
/// of the model implementation.

class Project {
  /// Using the singleton design pattern
 public:
  static Project& GetInstance() {
    static Project instance;  // Instantiated on first use.
    return instance;
  }

  ~Project() = default;

  /// Creates a new project
  /// @ param path path and filename of the project root file
  /// this file will contain a ProjectObject
  /// e.g. `/home/user/ProjectName.root`
  /// @return None
  void NewProject(const char* path, const char* name) {
    Log::Info("Creating new project at: ", path);
    Log::Info("  with name: ", name);

    fProjectObject.SetProjectName(name);
    fProjectPath.assign(path);
    fProjectObject.TestInit();
    fIsLoaded = kTRUE;

    SaveProject();
  }

  void CloseProject() {
    fProjectObject.Clear();
    fProjectPath.clear();
  }

  /// Loads project
  /// @param path path and filename of the project root file
  /// @return None
  const char* LoadProject(const char* path) {
    Log::Info("Loading project file: ", path);
    ReadProject(path);
    return fProjectObject.GetProjectName();
  }

  /// Saves Project and all of its models.
  /// @param modelName optional input to specify
  /// saving a single model. Default "" will save all models
  /// @return None
  void SaveProject() {
    Log::Info("Attempting to save project");
    try {
      Log::Info("Saving project to file: ", fProjectPath);
      bdm::WritePersistentObject(fProjectPath.c_str(), "ProjectObject",
                               fProjectObject, "recreate");
      Log::Info("Successfully saved!");
    } catch (...) {
      Log::Error("Couldn't save project!");
    }
    //SaveAllModels();
  }

  /// Saves Project under a different name. Copies over models from original
  /// project.
  /// @param path path and filename of the project root file
  /// @return None
  void SaveAsProject(std::string path) {
    Log::Info("Saving project as: ", path);
    fProjectPath.assign(path);
    SaveProject();
  }

  Bool_t CreateModel(const char* name) {
    Log::Info("Creating model within `Project`");
    Model* tmpModel = GetModel(name);
    if (tmpModel != nullptr) {
      Log::Warning("Cannot create model with the same name!");
      return kFALSE;
    } else {
      fProjectObject.CreateModel(name);
      return kTRUE;
    }
  }

  /// Creates model element for the specified model.
  /// @param modelName name of model to update
  /// @param parent parent model element that the new model element will belong to
  /// @param elementName name of the new element
  /// @param type type of element (e.g. M_ENTITY_CELL, M_MODULE_GROWTH, M_GENERAL_VARIABLE, etc.) 
  /// please see constants.h
  /// @return Bool_t kTRUE if successfully created model element, else kFALSE
  Bool_t CreateModelElement(const char* modelName, const char* parent,
                            const char* elementName, int type) {
    Model* modelPtr = GetModel(modelName);
    if (modelPtr == nullptr) {
      Log::Warning("Cannot create element `", elementName,
                   "` within non-existent model `", modelName, "`");
      return kFALSE;
    }
    return modelPtr->CreateElement(parent, elementName, type);
  }

  /// Gets model by name
  /// @param name name of the model to retrieve
  /// @return Model* pointer to the model of the specified name if found, else nullptr
  Model* GetModel(const char* name) {
    return fProjectObject.GetModel(name);
  }

  ModelElement* GetModelElement(const char* modelName, const char* elementName) {
    Model* tmpModel = GetModel(modelName);
    return tmpModel->GetModelElement(elementName);
  }

  /// Returns vector of all models
  std::vector<Model>* GetAllModels() { return fProjectObject.GetModels(); }

  void SaveAllModels() {
    /// TODO:
    //for(Model* model : fModels) { 
    //  bdm::WritePersistentObject(fProjectPath.c_str(), "ProjectObject",
    //                           fProjectObject, "recreate");
    //}
  }

  /// Starts BioDynaMo simulation on user-defined models.
  /// Initially it calls GenerateCode, the runs the command
  /// `biodynamo run` or `biodynamo build` on the generated code.
  /// @param simType signifies whether building or running.
  /// @param modelName optional input to specify simulation a
  /// single model. Default "" simulates all models.
  /// @return None
  void SimulateModel(SimulationType simType, const char* modelName = "") {}

  std::string GetCodeGenerateFolder(const char* modelName) {
    Model* model = GetModel(modelName);
    std::string folder(model->GetModelFolder());
    return folder;
  }

  std::string GetBackupFile(const char* modelName) {
    Model* model = GetModel(modelName);
    std::string filename(model->GetBackupFile());
    return filename;
  }

  /// Starts BioDynaMo code generation on user-defined models.
  /// The `Project` class description explains where code will reside.
  /// @param modelName optional input to specify generate code for
  /// a single model. Default "" generates for all models.
  /// @return None
  std::string GenerateCode(const char* modelName) {
    if(strcmp(modelName, "") != 0) {
      Model* model = GetModel(modelName);
      model->GenerateCode();
    } else {
      Log::Error("Cannot generate code for model:`", modelName, "`");
    }
    return "";
  }

  std::string GetProjectPath() { 
    std::string pathWithoutFilename(fProjectPath);
    pathWithoutFilename = pathWithoutFilename.erase(pathWithoutFilename.find_last_of("/") + 1);
    Log::Debug("Path without filename: ", pathWithoutFilename);
    return pathWithoutFilename;
  }

  Bool_t IsLoaded() { return fIsLoaded; }

  void SetSimulation(int argc, const char** argv) {
    currentSimulation = std::make_unique<bdm::Simulation>(argc, argv);
  }

  bdm::Simulation* GetSimulation() {
    return currentSimulation.get();
  }

  void PrintProjectDetails() {
    fProjectObject.PrintData();
  }

 private:
  /// Constructor
  Project() : fIsLoaded(kFALSE) {}
  Project(Project const&) = delete;
  Project& operator=(Project const&) = delete;

  void ReadProject(const char* path) {
    Log::Info("Attempting to load project");
    try {
      Log::Info("Reading project ...");
      ProjectObject *obj;
      bdm::GetPersistentObject(path, "ProjectObject", obj);
      Log::Info("Successfully Loaded!");
      /// TODO: a more elegant copy
      fProjectObject = (*obj);
      PrintProjectDetails();
      fIsLoaded = kTRUE;
      fProjectPath.assign(path);
    } catch (...) {
      Log::Error("Couldn't load project!");
    }
  }

  void WriteProject() {

  }

  std::string RunCmd(const char* cmd) {
    Log::Info("Running cmd:", cmd);
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
      throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      std::string tmp = buffer.data();
      Log::Info(tmp);
      result += tmp;
    }
    return result;
  }

  /// Contains all files and subfolders relevant to the project
  TDirectory fProjectDirectory;

  /// To be contained within fProjectFile
  ProjectObject fProjectObject;
  std::string fProjectPath;

  Bool_t fIsLoaded = kFALSE;  // kTRUE upon successful loading or new project creation

  std::unique_ptr<bdm::Simulation> currentSimulation;
};

}  // namespace gui

#endif  // GUI_PROJECT_H_