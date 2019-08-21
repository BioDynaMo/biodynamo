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

#include <Riostream.h>
#include <stdlib.h>
#include <time.h>
#include <stdexcept>
#include <string>

#include "gui/view/about.h"
#include "gui/view/button_model.h"
#include "gui/view/button_project.h"
#include "gui/constants.h"
#include "gui/controller/project.h"
#include "gui/view/grid_dialog.h"
#include "gui/view/help_text.h"
#include "gui/view/log.h"
#include "gui/view/model_creator.h"
#include "gui/view/model_frame.h"
#include "gui/view/new_dialog.h"
#include "gui/view/title.h"
#include "gui/view/tree_manager.h"

#include "biodynamo.h"
#include "core/simulation_backup.h"
#include "gui/diffusion_modules.h"

namespace gui {

const char *icon_names[] = {"new_project.png",
                            "",
                            "open.png",
                            "save.png",
                            "settings.png",
                            "",
                            "generate_code.png",
                            "",
                            "browser.png",
                            "",
                            "user_guide.png",
                            "dev_guide.png",
                            "license.png",
                            "about.png",
                            "",
                            "quit.png",
                            0};

ToolBarData_t tb_data[] = {
    {"", "New Project", kFALSE, M_FILE_NEWPROJECT, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Open Root project file", kFALSE, M_FILE_OPENPROJECT, NULL},
    {"", "Save project in Root file", kFALSE, M_FILE_SAVE, NULL},
    {"", "Project Preferences", kFALSE, M_FILE_PREFERENCES, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Generate BioDynaMo code", kFALSE, M_SIMULATION_GENERATE, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Start Root browser", kFALSE, M_TOOLS_STARTBROWSER, NULL},
    {"", 0, 0, -1, NULL},
    {"", "User Guide", kFALSE, M_HELP_USERGUIDE, NULL},
    {"", "Developer Guide", kFALSE, M_HELP_DEVGUIDE, NULL},
    {"", "Display License", kFALSE, M_HELP_LICENSE, NULL},
    {"", "About Model Creator", kFALSE, M_HELP_ABOUT, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Exit Application", kFALSE, M_FILE_EXIT, NULL},
    {NULL, NULL, 0, 0, NULL}};

const char *filetypes[] = {"ROOT files", "*.root", "All files", "*", 0, 0};

ModelCreator *gModelCreator;

Int_t ModelCreator::fgDefaultXPosition = 20;
Int_t ModelCreator::fgDefaultYPosition = 20;

////////////////////////////////////////////////////////////////////////////////
/// Create (the) Model Display.
///
/// p = pointer to GMainFrame (not owner)
/// w = width of ModelCreator frame
/// h = width of ModelCreator frame

ModelCreator::ModelCreator(const TGWindow *p, UInt_t w, UInt_t h)
    : TGMainFrame(p, w, h) {
  // Project::GetInstance();
  fOk = kFALSE;
  fModified = kFALSE;
  fIsRunning = kFALSE;
  fIsNewProject = kFALSE;
  fIsGridSet = kFALSE;

  fModelCreatorEnv = std::make_unique<TEnv>(".modelcreatorrc");  //  not yet used

  fTreeManager = std::make_unique<TreeManager>();

  fProjectName.clear();
  fProjectPath.clear();
  fModelName.clear();

  /// Create menubar and popup menus.
  MakeMenuBarFrame();

  ///---- toolbar
  int spacing = 8;
  fToolBar = std::make_unique<TGToolBar>(this, 60, 20, kHorizontalFrame | kRaisedFrame);
  for (uint32_t i = 0; icon_names[i]; i++) {
    std::string iconName = bdm::Concat(gProgPath, "/icons/", icon_names[i]);
    tb_data[i].fPixmap = iconName.c_str();
    if (strlen(icon_names[i]) == 0) {
      fToolBar->AddFrame(new TGVertical3DLine(fToolBar.get()),
                         new TGLayoutHints(kLHintsExpandY, 4, 4));
      continue;
    }
    const TGPicture *pic = fClient->GetPicture(tb_data[i].fPixmap);
    TGPictureButton *pb = new TGPictureButton(fToolBar.get(), pic, tb_data[i].fId);
    pb->SetToolTipText(tb_data[i].fTipText);
    tb_data[i].fButton = pb;

    fToolBar->AddButton(this, pb, spacing);
    spacing = 0;
  }
  AddFrame(fToolBar.get(),
           new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 0, 0, 0));
  fToolBar->GetButton(M_FILE_SAVE)->SetState(kButtonDisabled);

  /// Layout hints
  fL1 = std::make_unique<TGLayoutHints>(kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0);
  fL2 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
  fL3 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsExpandX, 0, 0, 0, 0);

  // CREATE TITLE FRAME
  fTitleFrame = std::make_unique<TitleFrame>(this, "BioDyanMo", "Model Creator", 100, 100);
  AddFrame(fTitleFrame.get(), fL1.get());

  // CREATE MAIN FRAME
  fMainFrame = std::make_unique<TGCompositeFrame>(this, 100, 100, kHorizontalFrame | kRaisedFrame);

  TGVerticalFrame *fV1 = new TGVerticalFrame(fMainFrame.get(), 150, 10, kSunkenFrame | kFixedWidth);

  TGLayoutHints *lo;

  lo = new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 2, 0, 2, 2);
  fMainFrame->AddFrame(fV1, lo);

  TGVSplitter *splitter = new TGVSplitter(fMainFrame.get(), 5);
  splitter->SetFrame(fV1, kTRUE);
  lo = new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 0, 0, 0, 0);
  fMainFrame->AddFrame(splitter, lo);

  lo = new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY, 0, 2,
                         2, 2);

  // Create Selection frame (i.e. with buttons and selection widgets)
  fSelectionFrame = std::make_unique<TGCompositeFrame>(fV1, 100, 100, kVerticalFrame);

  fButtonModelFrame = std::make_unique<ButtonModelFrame>(fSelectionFrame.get(), this, M_MODEL_NEW,
                                           M_MODEL_SIMULATE, M_CREATE_GRID);

  // create project button frame
  fButtonProjectFrame = std::make_unique<ButtonProjectFrame>(
      fSelectionFrame.get(), this, M_FILE_NEWPROJECT, M_FILE_OPENPROJECT);
  lo = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 2, 5, 1,
                         2);

  fSelectionFrame->AddFrame(fButtonProjectFrame.get(), lo);
  fSelectionFrame->AddFrame(fButtonModelFrame.get(), lo);

  fTreeView =
      std::make_unique<TGCanvas>(fSelectionFrame.get(), 150, 10, kSunkenFrame | kDoubleBorder);
  fProjectListTree =
      std::make_unique<TGListTree>(fTreeView->GetViewPort(), 10, 10, kHorizontalFrame);
  fProjectListTree->SetCanvas(fTreeView.get());
  fProjectListTree->Associate(this);
  fTreeView->SetContainer(fProjectListTree.get());
  fSelectionFrame->AddFrame(fTreeView.get(), fL2.get());

  lo = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
  fV1->AddFrame(fSelectionFrame.get(), lo);

  // Create Display frame
  fModelFrame = std::make_unique<ModelFrame>(fMainFrame.get(), this);
  fModelFrame->EnableButtons(M_NONE_ACTIVE);
  fMainFrame->AddFrame(fModelFrame.get(), lo);

  // TODO: add more to frames/tabs/etc

  AddFrame(fMainFrame.get(), lo);

  // Create status bar
  Int_t parts[] = {45, 45, 10};
  fStatusBar = std::make_unique<TGStatusBar>(this, 50, 10, kHorizontalFrame);
  fStatusBar->SetParts(parts, 3);
  AddFrame(fStatusBar.get(), fL3.get());
  Log::SetStatusBar(fStatusBar.get());
  fStatusBar->SetText("Please create or load a project", 0);

  // Finish ModelCreator for display...
  SetWindowName("BioDynaMo Model Creator");
  SetIconName("BioDynaMo Model Creator");

  MapSubwindows();
  Resize();  // this is used here to init layout algorithm
  MapWindow();

  ChangeSelectionFrame(kFALSE);

  gSystem->Load("libTreeViewer");
  AddInput(kKeyPressMask | kKeyReleaseMask);
  gVirtualX->SetInputFocus(GetId());
  gModelCreator = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Create menubar and popup menus.

void ModelCreator::MakeMenuBarFrame() {
  /// layout hint items
  fMenuBarLayout = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  fMenuBarItemLayout = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
  fMenuBarHelpLayout = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsRight);
  fMenuBar = std::make_unique<TGMenuBar>(this, 1, 1, kHorizontalFrame | kRaisedFrame);

  /// Menu - File
  fMenuFile = std::make_unique<TGPopupMenu>(gClient->GetRoot());
  fMenuFile->AddEntry("&New Project", M_FILE_NEWPROJECT);
  fMenuFile->AddEntry("&Open Project...", M_FILE_OPENPROJECT);
  fMenuFile->AddEntry("&Save", M_FILE_SAVE);
  fMenuFile->AddEntry("S&ave as...", M_FILE_SAVEAS);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Import", M_FILE_IMPORT);
  fMenuFile->AddEntry("Export", M_FILE_EXPORT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Preferences", M_FILE_PREFERENCES);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("E&xit", M_FILE_EXIT);

  fMenuFile->DisableEntry(M_FILE_SAVE);
  fMenuFile->DisableEntry(M_FILE_SAVEAS);

  /// Menu - Simulation
  fMenuSimulation = std::make_unique<TGPopupMenu>(gClient->GetRoot());
  fMenuSimulation->AddEntry("Generate BioDynaMo code", M_SIMULATION_GENERATE);

  /// Menu - Tools
  fMenuTools = std::make_unique<TGPopupMenu>(gClient->GetRoot());
  fMenuTools->AddLabel("Tools...");
  fMenuTools->AddSeparator();
  fMenuTools->AddEntry("Start &Browser\tCtrl+B", M_TOOLS_STARTBROWSER);

  /// Menu - View
  fMenuView = std::make_unique<TGPopupMenu>(gClient->GetRoot());
  fMenuView->AddEntry("&Toolbar", M_VIEW_TOOLBAR);
  fMenuView->CheckEntry(M_VIEW_TOOLBAR);

  /// Menu - Samples
  fMenuSamples = std::make_unique<TGPopupMenu>(gClient->GetRoot());
  fMenuSamples->AddLabel("Try out demo projects...");
  fMenuSamples->AddSeparator();
  fMenuSamples->AddEntry("Cell Division", M_SAMPLES_CELLDIVISION);
  fMenuSamples->AddEntry("Diffusion", M_SAMPLES_DIFFUSION);
  fMenuSamples->AddEntry("Gene Regulation", M_SAMPLES_GENEREGULATION);
  fMenuSamples->AddEntry("Some Clustering", M_SAMPLES_SOMACLUSTERING);
  fMenuSamples->AddEntry("Tumor Concept", M_SAMPLES_TUMORCONCEPT);

  fMenuSamples->DisableEntry(M_SAMPLES_CELLDIVISION);
  fMenuSamples->DisableEntry(M_SAMPLES_DIFFUSION);
  fMenuSamples->DisableEntry(M_SAMPLES_GENEREGULATION);
  fMenuSamples->DisableEntry(M_SAMPLES_SOMACLUSTERING);
  fMenuSamples->DisableEntry(M_SAMPLES_TUMORCONCEPT);

  /// Menu - Help
  fMenuHelp = std::make_unique<TGPopupMenu>(gClient->GetRoot());
  fMenuHelp->AddEntry("User Guide", M_HELP_USERGUIDE);
  fMenuHelp->AddEntry("Dev Guide", M_HELP_DEVGUIDE);
  fMenuHelp->AddSeparator();
  fMenuHelp->AddEntry("About", M_HELP_ABOUT);

  /// Associate signals
  fMenuFile->Associate(this);
  fMenuSamples->Associate(this);
  fMenuTools->Associate(this);
  fMenuView->Associate(this);
  fMenuSimulation->Associate(this);
  fMenuHelp->Associate(this);
  fMenuFile->Associate(this);

  fMenuBar->AddPopup("&File", fMenuFile.get(), fMenuBarItemLayout.get());
  fMenuBar->AddPopup("S&imulation", fMenuSimulation.get(), fMenuBarItemLayout.get());
  fMenuBar->AddPopup("&Tools", fMenuTools.get(), fMenuBarItemLayout.get());
  fMenuBar->AddPopup("&View", fMenuView.get(), fMenuBarItemLayout.get());
  fMenuBar->AddPopup("&Samples", fMenuSamples.get(), fMenuBarItemLayout.get());
  fMenuBar->AddPopup("&Help", fMenuHelp.get(), fMenuBarHelpLayout.get());

  AddFrame(fMenuBar.get(), fMenuBarLayout.get());
}


////////////////////////////////////////////////////////////////////////////////
/// Show or hide toolbar.

void ModelCreator::ShowToolBar(Bool_t show) {
  if (show) {
    ShowFrame(fToolBar.get());
    fMenuView->CheckEntry(M_VIEW_TOOLBAR);
  } else {
    HideFrame(fToolBar.get());
    fMenuView->UnCheckEntry(M_VIEW_TOOLBAR);
  }
}

void ModelCreator::EnableSaving(Bool_t enable) {
  if (enable) {
    fMenuFile->EnableEntry(M_FILE_SAVE);
    fMenuFile->EnableEntry(M_FILE_SAVEAS);
    fToolBar->GetButton(M_FILE_SAVE)->SetState(kButtonUp);
  } else {
    fMenuFile->DisableEntry(M_FILE_SAVE);
    fMenuFile->DisableEntry(M_FILE_SAVEAS);
    fToolBar->GetButton(M_FILE_SAVE)->SetState(kButtonDisabled);
  }
  
}

////////////////////////////////////////////////////////////////////////////////
/// Set the default position on the screen of new Model Creator instances.

void ModelCreator::SetDefaultPosition(Int_t x, Int_t y) {
  fgDefaultXPosition = x;
  fgDefaultYPosition = y;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new Project within model creator,
/// both fProjectName and fProjectPath should be set.
void ModelCreator::CreateNewProject() {
  if (fProjectName.empty() || fProjectPath.empty()) {
    Log::Error("Project name or path is empty! Cannot create Project!");
    return;
  }
  Project::GetInstance().NewProject(fProjectPath.c_str(), fProjectName.c_str());
  /// Previous tree manager will be destroyed
  fTreeManager->CreateProjectTree(fProjectListTree.get(), fProjectName);
  Initialize();
  ChangeSelectionFrame();
}

void ModelCreator::LoadProject(std::string fileName) {
  const char* projectName = Project::GetInstance().LoadProject(fileName.c_str());
  const char* projectLocation = fileName.c_str();
  NewProjectSet(projectName, projectLocation);
  fTreeManager->CreateProjectTree(fProjectListTree.get(), fProjectName);
  std::vector<Model>* models = Project::GetInstance().GetAllModels();
  size_t modelCount = models->size();
  Log::Info("Number of models:", modelCount);
  for (uint32_t i = 0; i < modelCount; i++) {
    Model curModel = models->at(i);
    fTreeManager->CreateModelTree(curModel);
  }
  ChangeSelectionFrame();
}

Bool_t ModelCreator::AskForProject(Bool_t loading) {
  Log::Warning("Can only have 1 project open!");
  Int_t retval;
  std::string msg("Press OK to save the current project");
  if (loading) {
    msg.append(", then load an existing one.");
  } else {
    msg.append(", then create a new one.");
  }
  new TGMsgBox(gClient->GetRoot(), this,
              "Warning", msg.c_str(),
              kMBIconExclamation, kMBOk | kMBCancel, &retval);
  if (retval == kMBOk) {
    return kTRUE;
  }
  return kFALSE;
}

void ModelCreator::ClearProject() {
  if (Project::GetInstance().IsLoaded()) {
    Project::GetInstance().SaveProject();
    Project::GetInstance().CloseProject();
    fModelFrame->ClearTabs();
    VisManager::GetInstance().Reset();
  }
  fTreeManager = std::make_unique<TreeManager>();
  fProjectListTree->Cleanup();
  ChangeSelectionFrame(kFALSE);
  fModelFrame->EnableButtons(M_NONE_ACTIVE);
  fModelName.clear();
  fProjectName.clear();
  fProjectPath.clear();
}

/// Will create new model if a model with
/// the same name does not already exist
void ModelCreator::CreateNewModel() {
  if (Project::GetInstance().CreateModel(fModelName.c_str())) {
    Log::Info("Creating Model on tree:", fModelName);
    fTreeManager->CreateModelTree(fModelName);
    Initialize();
    fClient->NeedRedraw(fModelFrame.get());
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize ModelCreator display.

void ModelCreator::Initialize() {
  fProjectListTree->ClearViewPort();
  fClient->NeedRedraw(fProjectListTree.get());
  fStatusBar->SetText("", 1);
}

void ModelCreator::ChangeSelectionFrame(Bool_t createdProject) {
  fSelectionFrame->MapSubwindows();
  if (createdProject) {
    fSelectionFrame->ShowFrame(fButtonModelFrame.get());
    fSelectionFrame->HideFrame(fButtonProjectFrame.get());
  } else {
    fSelectionFrame->HideFrame(fButtonModelFrame.get());
    fSelectionFrame->ShowFrame(fButtonProjectFrame.get());
  }
  fSelectionFrame->MapWindow();
}

void ModelCreator::CreateNewElement(Int_t type) {
  fButtonModelFrame->SetState(M_ALL_ACTIVE);
  std::string elemName = fTreeManager->CreateTopLevelElement(type);
  Project::GetInstance().CreateModelElement(fModelName.c_str(), "", elemName.c_str(), type);
  fProjectListTree->ClearViewPort();
  fClient->NeedRedraw(fProjectListTree.get());
}

void ModelCreator::CreateGrid() {
  Log::Info("About to create grid!");
  fButtonModelFrame->SetState(M_ALL_ACTIVE);

  // Starting position
  double xPos = 0, yPos = 0, zPos = 0;
  
  Long_t x, y, z;

  for (z = 0; z < gridNumberZ; z++) {
    for (y = 0; y < gridNumberY; y++) {
      for (x = 0; x < gridNumberX; x++) {
        bdm::Double3 pos = {xPos, yPos, zPos};
        std::string elemName = fTreeManager->CreateTopLevelElement(M_ENTITY_CELL);
        Project::GetInstance().CreateGridCell(fModelName.c_str(), elemName.c_str(), pos);
        xPos += gridDistanceX;
      }
      xPos = 0;
      yPos += gridDistanceY;
    }
    yPos = 0;
    zPos += gridDistanceZ;
  }
  
  fProjectListTree->ClearViewPort();
  fClient->NeedRedraw(fProjectListTree.get());
}

Bool_t ModelCreator::GenerateModelCode() {
  std::string genFolder = Project::GetInstance().GetCodeGenerateFolder(fModelName.c_str());
  Int_t retval;
  std::string msg = bdm::Concat("This will overwrite all files in `", genFolder, "` Press OK to continue.");
  new TGMsgBox(gClient->GetRoot(), this,
              "Info", msg.c_str(),
              kMBIconExclamation, kMBOk | kMBCancel, &retval);
  if (retval != kMBOk) {
    return kFALSE;
  }
  Bool_t diffusionEnabled = fModelFrame->CheckAllSecretionBoxes();
  Project::GetInstance().GenerateCode(fModelName.c_str(), diffusionEnabled);
  return kTRUE;
}

void ModelCreator::SimulateModel() {
  /// First: Generate code
  if (!GenerateModelCode()) {
    Log::Info("Cannot simulate model without generating code!");
    return;
  }
  
  std::string modelFolder = Project::GetInstance().GetCodeGenerateFolder(fModelName.c_str()); 

  /// Second: Run the simulation with expected backup
  std::string currentDir = RunCmd("pwd");
  //std::string backupFilepath = Project::GetInstance().GetBackupFile(fModelName.c_str()); 
  std::string backupFilepath("guibackup.root"); 

  Log::Debug("BackupFile: ", backupFilepath);
  std::string firstCmd = bdm::Concat("cd ", modelFolder, " && biodynamo build && cd build && ./", 
                                     fModelName.c_str(), " -b ", backupFilepath.c_str());
  std::string secondCmd = bdm::Concat("cd ", currentDir);

  VisManager::GetInstance().Enable(kFALSE);

  std::string firstResult = RunCmd(firstCmd.c_str());

  Log::Info(firstResult);

  backupFilepath = Project::GetInstance().GetBackupFile(fModelName.c_str()); 

  ifstream f(backupFilepath.c_str());
  Bool_t backupExists = f.good();
  f.close();
  
  /// Third: Restore the backup into this environment
  if (backupExists) {
    VisManager::GetInstance().Enable();
    Log::Debug("Restoring simulation...");
    //std::string backupPathFull = Project::GetInstance().GetBackupFile(fModelName.c_str());
    const char* argv[] = { fModelName.c_str(), "-r", backupFilepath.c_str(), NULL };
    Int_t argc = sizeof(argv)/sizeof(argv[0]) - 1;

    /// TODO: max_bound is dynamically dependant on pre-existing cells
    auto set_param = [](bdm::Param* param) {
      param->bound_space_ = true;
      param->min_bound_ = 0;
      param->max_bound_ = 10000;  // cube of 10000*10000*10000
    };

    VisManager::GetInstance().Reset();
    std::string secondResult = RunCmd(secondCmd.c_str());

    /// Fourth: Set and start the simulation
    Project::GetInstance().SetSimulation(argc, argv, set_param);
    bdm::Simulation *currentSim = Project::GetInstance().GetSimulation();

    // If diffusion is enabled, simulate for 500 timesteps
    if (fModelFrame->CheckAllSecretionBoxes()) {
      currentSim->GetScheduler()->Simulate(500);
    }
    else {
      currentSim->GetScheduler()->Simulate(2);
    }
  } else {
    Log::Debug("backupfile does not exist! Simulation did not run properly!");
    Log::Error("Error occured during backupfile generation.");
    std::string secondResult = RunCmd(secondCmd.c_str());
  }
}

std::string ModelCreator::RunCmd(const char* cmd) {
  Log::Info("Running cmd:", cmd);
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
      throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      std::string tmp = buffer.data();
      result += tmp;
  }
  return result;
}

void ModelCreator::OpenWebpage(const char* url) {
  std::string cmd = bdm::Concat("firefox ", url, " &");
  Log::Info("Running`", cmd, "`");
  int retVal = system(cmd.c_str());
  if(retVal != 0) {
    Log::Error("Could not open firefox!");
  }
}

void ModelCreator::SetGrid(Long_t numberX, Long_t numberY, Long_t numberZ, 
                           Long_t distanceX, Long_t distanceY, Long_t distanceZ) {
  Log::Debug("Setting grid variables in ModelCreator!");
  fIsGridSet = kTRUE;
  gridNumberX = numberX;
  gridNumberY = numberY;
  gridNumberZ = numberZ;
  gridDistanceX = distanceX;
  gridDistanceY = distanceY;
  gridDistanceZ = distanceZ;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle messages send to the ModelCreator object.

Bool_t ModelCreator::ProcessMessage(Long_t msg, Long_t param1, Long_t param2) {

  switch (GET_MSG(msg)) {
    case kC_COMMAND:
      switch (GET_SUBMSG(msg)) {
        case kCM_BUTTON:
        case kCM_MENU:
          switch (param1) {
            case M_FILE_NEWPROJECT: {
              if (Project::GetInstance().IsLoaded()) {
                if (AskForProject()) {
                  goto NewProject;
                }
              } else {
                NewProject:
                ClearProject();
                new NewProjectDialog(fClient->GetRoot(), this, 800, 400);
                CreateNewProject();
                EnableSaving();
                fModified = kTRUE;
              }
              break;
            }

            case M_FILE_OPENPROJECT:
            {
              if (Project::GetInstance().IsLoaded()) {
                if (AskForProject(kTRUE)) {
                  ClearProject();
                  goto LoadingProject;
                }
              } else {
                LoadingProject:
                Log::Debug("Clicked open project!");
                TGFileInfo fi;
                fi.fFileTypes = filetypes;
                new TGFileDialog(fClient->GetRoot(), this, kFDOpen, &fi);
                if (!fi.fFilename) return kTRUE;
                LoadProject(fi.fFilename);
                EnableSaving();
              }
            }
              break;

            case M_MODEL_NEW:
              new NewModelDialog(fClient->GetRoot(), this, 800, 400);
              if (fIsNewModel) {
                fModified = kTRUE;
                fIsNewModel = kFALSE;
                CreateNewModel();
              }
              break;

            case M_MODEL_SIMULATE:
              Log::Debug("Clicked simulate!");
              SimulateModel();
              break;

            case M_CREATE_GRID:
              Log::Debug("Clicked create grid!");
              new GridDialog(gClient->GetRoot(), this, 800, 400);
              if (fIsGridSet) {
                fIsGridSet = kFALSE;
                CreateGrid();
              }
              break;

            case M_SIMULATION_GENERATE:
            {
              if (!fModelName.empty()) {
                Bool_t isGenerated = GenerateModelCode();
                if (isGenerated) {
                  Log::Info("Successfully generated biodynamo code!");
                }
              } else {
                Log::Info("Please create a model first!");
              }
            }
              break;

            case M_FILE_SAVE:
              Log::Debug("Clicked save!");
              Project::GetInstance().SaveProject();
              fModified = kFALSE;
              break;

            case M_FILE_SAVEAS: 
            {
              Log::Debug("Clicked save as!");
              TGFileInfo fi;
              fi.fFileTypes = filetypes;
              new TGFileDialog(fClient->GetRoot(), this, kFDOpen, &fi);
              if (!fi.fFilename) return kTRUE;
              Project::GetInstance().SaveAsProject(fi.fFilename);
            }
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            case M_FILE_PREFERENCES:
              Log::Info("Clicked preferences!");
              break;

            case M_ENTITY_CELL:
              Log::Debug("Clicked cell!");
              CreateNewElement(M_ENTITY_CELL);
              fModified = kTRUE;
              break;

            case M_MODULE_GROWTH:
              Log::Debug("Clicked growth module!");
              CreateNewElement(M_MODULE_GROWTH);
              fModified = kTRUE;
              break;

            case M_MODULE_CHEMOTAXIS:
              Log::Debug("Clicked chemotaxis module!");
              fModified = kTRUE;
              break;

            case M_MODULE_SUBSTANCE:
              Log::Debug("Clicked substance secretion module!");
              fModified = kTRUE;
              break;

            case M_GENERAL_VARIABLE:
              Log::Debug("Clicked general variable!");
              fModified = kTRUE;
              break;

            case M_GENERAL_FUNCTION:
              Log::Debug("Clicked general function!");
              fModified = kTRUE;
              break;

            case M_GENERAL_FORMULA:
              Log::Debug("Clicked general formula!");
              fModified = kTRUE;
              break;

            case M_TOOLS_STARTBROWSER:
            {
              new TBrowser;
            }
              break;

            case M_VIEW_TOOLBAR:
              if (fMenuView->IsEntryChecked(M_VIEW_TOOLBAR))
                ShowToolBar(kFALSE);
              else
                ShowToolBar();
              break;

            case M_HELP_LICENSE:
              int ax, ay;
              TRootHelpDialog *hd;
              Window_t wdummy;
              Char_t strtmp[250];
              sprintf(strtmp, "Model Creator License");
              hd = new TRootHelpDialog(this, strtmp, 640, 380);
              hd->SetText(gHelpLicense);
              gVirtualX->TranslateCoordinates(
                  GetId(), GetParent()->GetId(), (Int_t)(GetWidth() - 640) >> 1,
                  (Int_t)(GetHeight() - 380) >> 1, ax, ay, wdummy);
              hd->Move(ax, ay);
              hd->Popup();
              fClient->WaitFor(hd);
              break;

            case M_HELP_USERGUIDE:
              OpenWebpage("https://biodynamo.github.io/user/");
              break;

            case M_HELP_DEVGUIDE:
              OpenWebpage("https://biodynamo.github.io/dev/");
              break;

            case M_HELP_ABOUT:
              new ModelCreatorAbout(gClient->GetRoot(), this, 400, 200);
              break;

          }       // switch param1
          break;  // M_MENU
      }       // switch submsg
      break;  // case kC_COMMAND

    case kC_LISTTREE:
      switch (GET_SUBMSG(msg)) {
        case kCT_ITEMDBLCLICK:
          if (param1 == kButton1) {
            if (fProjectListTree->GetSelected()) {
              fProjectListTree->ClearViewPort();
              fClient->NeedRedraw(fProjectListTree.get());
            }
          }
          break;

        case kCT_ITEMCLICK:
          HandleTreeInput();
          break;

      }       // switch submsg
      break;  // case kC_LISTTREE
  }           // switch msg

  return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Process mouse clicks in TGListTree.

void ModelCreator::HandleTreeInput() {
  fProjectListTree->ClearViewPort();
  std::string selectedModelName = fTreeManager->IsModelSelected();
  if (!selectedModelName.empty()) {
    Log::Info("Selected part of ", selectedModelName);
    fModelName.assign(selectedModelName);
    fModelFrame->EnableButtons(M_ALL_ACTIVE);
    fButtonModelFrame->SetState(M_GRID_ACTIVE);
  } else {
    fModelFrame->EnableButtons(M_NONE_ACTIVE);
  }
  std::string itemName(fProjectListTree->GetSelected()->GetText());
  if (itemName.find("Cell") != std::string::npos ||
      itemName.find("Growth") != std::string::npos) {
    fModelFrame->ShowModelElement(selectedModelName.c_str(),
                                  itemName.c_str());
    fButtonModelFrame->SetState(M_ALL_ACTIVE);
    fClient->NeedRedraw(fModelFrame.get());
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Got close message for this Model Creator. The ModelDislay and the
/// application will be terminated.

void ModelCreator::CloseWindow() {
  if (fModified) {
    Int_t retval;
    new TGMsgBox(gClient->GetRoot(), this,
                "Info", "Any unsaved changes will be lost! Press OK to continue.",
                kMBIconExclamation, kMBOk | kMBCancel, &retval);
    if (retval != kMBOk) {
      return;
    }
  }

  std::cout << "Terminating Model Creator" << std::endl;

  //this->DeleteWindow();
  gApplication->Terminate();
}

}  // namespace gui