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
// This File contains the implementation of the ModelCreator-class      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <Riostream.h>
#include <stdlib.h>
#include <time.h>
#include <stdexcept>
#include <string>

#include <KeySymbols.h>
#include <TEnv.h>
#include <TROOT.h>
#include <TRint.h>
#include <TStyle.h>
#include <TVirtualX.h>

#include <TF1.h>
#include <TFile.h>
#include <TFrame.h>
#include <TH1.h>
#include <TTree.h>

#include <TBrowser.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TContextMenu.h>
#include <TG3DLine.h>
#include <TGButton.h>
#include <TGFileDialog.h>
#include <TGListTree.h>
#include <TGMenu.h>
#include <TGSplitter.h>
#include <TGStatusBar.h>
#include <TGTab.h>
#include <TGTextEdit.h>
#include <TGToolBar.h>
#include <TGToolTip.h>
#include <TGeoManager.h>
#include <THtml.h>
#include <TParticle.h>
#include <TRandom.h>
#include <TRootEmbeddedCanvas.h>
#include <TRootHelpDialog.h>
#include <TSystem.h>
#include <TView.h>

#include <TPluginManager.h>
#include <TVirtualGL.h>

#include "about.h"
#include "button_model.h"
#include "button_project.h"
#include "gui/constants.h"
#include "gui/controller/project.h"
#include "help_text.h"
#include "log.h"
#include "model_creator.h"
#include "model_frame.h"
#include "new_dialog.h"
#include "title.h"
#include "tree_manager.h"

namespace gui {

const char *icon_names[] = {"new_project.xpm",
                            "",
                            "open.xpm",
                            "save.xpm",
                            "settings.xpm",
                            "",
                            "build.png",
                            "run.png",
                            "generate_code.xpm",
                            "",
                            "browser.xpm",
                            "",
                            "user_guide.xpm",
                            "dev_guide.xpm",
                            "license.xpm",
                            "about.xpm",
                            "",
                            "quit.xpm",
                            0};

ToolBarData_t tb_data[] = {
    {"", "New Project", kFALSE, M_FILE_NEWPROJECT, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Open Root project file", kFALSE, M_FILE_OPENPROJECT, NULL},
    {"", "Save project in Root file", kFALSE, M_FILE_SAVE, NULL},
    {"", "Project Preferences", kFALSE, M_FILE_PREFERENCES, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Build current model", kFALSE, M_SIMULATION_BUILD, NULL},
    {"", "Run current model", kFALSE, M_SIMULATION_RUN, NULL},
    {"", "Generate BioDynaMo code", kFALSE, M_SIMULATION_GENERATE, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Start Root browser", kFALSE, M_TOOLS_STARTBROWSER, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Developer Guide", kFALSE, M_HELP_USERGUIDE, NULL},
    {"", "User Guide", kFALSE, M_HELP_DEVGUIDE, NULL},
    {"", "Display License", kFALSE, M_HELP_LICENSE, NULL},
    {"", "About Model Creator", kFALSE, M_HELP_ABOUT, NULL},
    {"", 0, 0, -1, NULL},
    {"", "Exit Application", kFALSE, M_FILE_EXIT, NULL},
    {NULL, NULL, 0, 0, NULL}};

const char *filetypes[] = {"ROOT files", "*.root", "All files", "*", 0, 0};

TGListTree *gProjectListTree;  // base TGListTree

ModelCreator *gModelCreator;

//_________________________________________________
// ModelCreator
//

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
  fSettingsModified = kFALSE;
  fIsRunning = kFALSE;
  fInterrupted = kFALSE;
  fIsNewProject = kFALSE;

  fModelCreatorEnv =
      new TEnv(".modelcreatorrc");  // fModelCreatorEnv not yet used

  fTreeManager = new TreeManager();

  fProjectName.clear();
  fProjectPath.clear();

  /// Create menubar and popup menus.
  MakeMenuBarFrame();

  ///---- toolbar
  int spacing = 8;
  fToolBar = new TGToolBar(this, 60, 20, kHorizontalFrame | kRaisedFrame);
  for (int i = 0; icon_names[i]; i++) {
    TString iconname(gProgPath);
#ifdef R__WIN32
    iconname += "\\icons\\";
#else
    iconname += "/icons/";
#endif
    iconname += icon_names[i];
    tb_data[i].fPixmap = iconname.Data();
    if (strlen(icon_names[i]) == 0) {
      fToolBar->AddFrame(new TGVertical3DLine(fToolBar),
                         new TGLayoutHints(kLHintsExpandY, 4, 4));
      continue;
    }
    const TGPicture *pic = fClient->GetPicture(tb_data[i].fPixmap);
    TGPictureButton *pb = new TGPictureButton(fToolBar, pic, tb_data[i].fId);
    pb->SetToolTipText(tb_data[i].fTipText);
    tb_data[i].fButton = pb;

    fToolBar->AddButton(this, pb, spacing);
    spacing = 0;
  }
  AddFrame(fToolBar,
           new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 0, 0, 0));
  fToolBar->GetButton(M_FILE_SAVE)->SetState(kButtonDisabled);

  /// Layout hints
  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  fL2 = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0);
  fL3 = new TGLayoutHints(
      kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0);
  fL4 =
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 5, 2, 2);
  fL5 = new TGLayoutHints(
      kLHintsBottom | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2,
      2);
  fL6 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 0, 0, 0, 0);
  fL7 = new TGLayoutHints(
      kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 2, 2);
  fL8 =
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);

  // CREATE TITLE FRAME
  fTitleFrame = new TitleFrame(this, "BioDyanMo", "Model Creator", 100, 100);
  AddFrame(fTitleFrame, fL2);

  // CREATE MAIN FRAME
  fMainFrame =
      new TGCompositeFrame(this, 100, 100, kHorizontalFrame | kRaisedFrame);

  TGVerticalFrame *fV1 =
      new TGVerticalFrame(fMainFrame, 150, 10, kSunkenFrame | kFixedWidth);

  TGLayoutHints *lo;

  lo = new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 2, 0, 2, 2);
  fMainFrame->AddFrame(fV1, lo);

  TGVSplitter *splitter = new TGVSplitter(fMainFrame, 5);
  splitter->SetFrame(fV1, kTRUE);
  lo = new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 0, 0, 0, 0);
  fMainFrame->AddFrame(splitter, lo);

  lo = new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY, 0, 2,
                         2, 2);

  // Create Selection frame (i.e. with buttons and selection widgets)
  fSelectionFrame = new TGCompositeFrame(fV1, 100, 100, kVerticalFrame);

  fButtonModelFrame = new ButtonModelFrame(fSelectionFrame, this, M_MODEL_NEW,
                                           M_MODEL_SIMULATE, M_INTERRUPT_SIMUL);

  // create project button frame
  fButtonProjectFrame = new ButtonProjectFrame(
      fSelectionFrame, this, M_FILE_NEWPROJECT, M_FILE_OPENPROJECT);
  lo = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 2, 5, 1,
                         2);

  fSelectionFrame->AddFrame(fButtonProjectFrame, lo);
  fSelectionFrame->AddFrame(fButtonModelFrame, lo);

  fTreeView =
      new TGCanvas(fSelectionFrame, 150, 10, kSunkenFrame | kDoubleBorder);
  fProjectListTree =
      new TGListTree(fTreeView->GetViewPort(), 10, 10, kHorizontalFrame);
  gProjectListTree = fProjectListTree;
  fProjectListTree->SetCanvas(fTreeView);
  fProjectListTree->Associate(this);
  fTreeView->SetContainer(fProjectListTree);
  fSelectionFrame->AddFrame(fTreeView, fL5);

  lo = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
  fV1->AddFrame(fSelectionFrame, lo);

  fContextMenu = new TContextMenu("RSContextMenu");

  // Create Display frame
  fModelFrame = new ModelFrame(fMainFrame, this);
  fModelFrame->EnableButtons(M_NONE_ACTIVE);
  fMainFrame->AddFrame(fModelFrame, lo);

  // Create Display Canvas Tab (where the actual models are displayed)
  // TGCompositeFrame *tFrame = fDisplayFrame->AddTab("Untitled Model");

  // TODO: add more to frames/tabs/etc

  AddFrame(fMainFrame, lo);

  // Create status bar
  Int_t parts[] = {45, 45, 10};
  fStatusBar = new TGStatusBar(this, 50, 10, kHorizontalFrame);
  fStatusBar->SetParts(parts, 3);
  AddFrame(fStatusBar, fL6);
  fStatusBar->SetText("Waiting to start simulation...", 0);

  // Finish ModelCreator for display...
  SetWindowName("BioDynaMo Model Creator");
  SetIconName("BioDynaMo Model Creator");

  MapSubwindows();
  Resize();  // this is used here to init layout algorithm
  MapWindow();

  ChangeSelectionFrame(kFALSE);

  // fEvent = new MyEvent();
  // fEvent->GetDetector()->Init();
  // fEvent->Init(0, fFirstParticle, fE0, fB);
  // Initialize();
  // gROOT->GetListOfBrowsables()->Add(fEvent,"RootShower Event");
  gSystem->Load("libTreeViewer");
  AddInput(kKeyPressMask | kKeyReleaseMask);
  gVirtualX->SetInputFocus(GetId());
  gModelCreator = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Create menubar and popup menus.

void ModelCreator::MakeMenuBarFrame() {
  /// layout hint items
  fMenuBarLayout =
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
  fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);
  fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame | kRaisedFrame);

  /// Menu - File
  fMenuFile = new TGPopupMenu(gClient->GetRoot());
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
  fMenuSimulation = new TGPopupMenu(gClient->GetRoot());
  fMenuSimulation->AddEntry("Generate BioDynaMo code", M_SIMULATION_GENERATE);
  fMenuSimulation->AddSeparator();
  fMenuSimulation->AddEntry("Build", M_SIMULATION_BUILD);
  fMenuSimulation->AddEntry("Run", M_SIMULATION_RUN);
  fMenuSimulation->AddSeparator();
  fMenuSimulation->AddEntry("Open Paraview", M_SIMULATION_OPENPARAVIEW);

  fMenuSimulation->DisableEntry(M_SIMULATION_GENERATE);
  fMenuSimulation->DisableEntry(M_SIMULATION_BUILD);
  fMenuSimulation->DisableEntry(M_SIMULATION_RUN);
  fMenuSimulation->DisableEntry(M_SIMULATION_OPENPARAVIEW);

  /// Menu - Tools
  fMenuTools = new TGPopupMenu(gClient->GetRoot());
  fMenuTools->AddLabel("Tools...");
  fMenuTools->AddSeparator();
  fMenuTools->AddEntry("Start &Browser\tCtrl+B", M_TOOLS_STARTBROWSER);

  /// Menu - View
  fMenuView = new TGPopupMenu(gClient->GetRoot());
  fMenuView->AddEntry("&Toolbar", M_VIEW_TOOLBAR);
  fMenuView->CheckEntry(M_VIEW_TOOLBAR);

  /// Menu - Samples
  fMenuSamples = new TGPopupMenu(gClient->GetRoot());
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
  fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("User Guide", M_HELP_USERGUIDE);
  fMenuHelp->AddEntry("Dev Guide", M_HELP_DEVGUIDE);
  fMenuHelp->AddSeparator();
  fMenuHelp->AddEntry("About", M_HELP_ABOUT);

  fMenuHelp->DisableEntry(M_HELP_USERGUIDE);
  fMenuHelp->DisableEntry(M_HELP_DEVGUIDE);

  /// Associate signals
  fMenuFile->Associate(this);
  fMenuSamples->Associate(this);
  fMenuTools->Associate(this);
  fMenuView->Associate(this);
  fMenuSimulation->Associate(this);
  fMenuHelp->Associate(this);
  fMenuFile->Associate(this);

  fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
  fMenuBar->AddPopup("S&imulation", fMenuSimulation, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Tools", fMenuTools, fMenuBarItemLayout);
  fMenuBar->AddPopup("&View", fMenuView, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Samples", fMenuSamples, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

  AddFrame(fMenuBar, fMenuBarLayout);
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy menubar and popup menus.

void ModelCreator::CloseMenuBarFrame() {
  delete fMenuHelp;
  delete fMenuSamples;
  delete fMenuView;
  delete fMenuTools;
  delete fMenuSimulation;
  delete fMenuFile;

  delete fMenuBarItemLayout;
  delete fMenuBarHelpLayout;
  delete fMenuBar;
  delete fMenuBarLayout;
}

////////////////////////////////////////////////////////////////////////////////
/// Show or hide toolbar.

void ModelCreator::ShowToolBar(Bool_t show) {
  if (show) {
    ShowFrame(fToolBar);
    fMenuView->CheckEntry(M_VIEW_TOOLBAR);
  } else {
    HideFrame(fToolBar);
    fMenuView->UnCheckEntry(M_VIEW_TOOLBAR);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy ModelCreator object. Delete all created widgets
/// GUI MEMBERS
ModelCreator::~ModelCreator() {
  CloseMenuBarFrame();
  delete fContextMenu;
  delete fButtonModelFrame;
  delete fButtonProjectFrame;
  delete fSelectionFrame;
  delete fMainFrame;
  delete fTitleFrame;

  delete fL8;
  delete fL7;
  delete fL6;
  delete fL5;
  delete fL4;
  delete fL3;
  delete fL2;
  //delete fL1;

  delete fProjectListTree;
  delete fTreeView;
  delete fTreeManager;
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
  fTreeManager->CreateProjectTree(fProjectListTree, fProjectName);
  Initialize();
  ChangeSelectionFrame();
}

void ModelCreator::CreateNewModel() {
  std::string teststr(fModelName);
  Log::Info("Creating Model:", fModelName);
  fTreeManager->CreateModelTree(fModelName);
  Initialize();
  std::string tmp = fModelName + " Overview";
  fModelFrame->ShowModelElement(fModelName.c_str(), tmp.c_str());
  fClient->NeedRedraw(fModelFrame);
  Project::GetInstance().CreateModel(fModelName.c_str());
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize ModelCreator display.

void ModelCreator::Initialize() {
  Interrupt(kFALSE);
  fProjectListTree->ClearViewPort();
  fClient->NeedRedraw(fProjectListTree);

  fStatusBar->SetText("", 1);
}

void ModelCreator::ChangeSelectionFrame(Bool_t createdProject) {
  fSelectionFrame->MapSubwindows();
  if (createdProject) {
    fSelectionFrame->ShowFrame(fButtonModelFrame);
    fSelectionFrame->HideFrame(fButtonProjectFrame);
  } else {
    fSelectionFrame->HideFrame(fButtonModelFrame);
    fSelectionFrame->ShowFrame(fButtonProjectFrame);
  }
  fSelectionFrame->MapWindow();
}

void ModelCreator::CreateNewCell() {
  std::string currentModel = fTreeManager->GetCurrentSelectedModelName();
  // std::string currentElement = TODO: GetCurrentSleectedModelElement
  // Will assume the top-level element folder for now
  Log::Info("Creating cell!");
  // Bool_t created =
  // Project::GetInstance().CreateModelElement(fModelName.c_str(), "", "Cell1",
  // M_ENTITY_CELL); if(created) {
  //  /// Call tree manager for update
  fTreeManager->CreateTopLevelElement(M_ENTITY_CELL);
  fProjectListTree->ClearViewPort();
  fClient->NeedRedraw(fProjectListTree);
  //} else {
  //   Log::Error("Unable to create new cell!");
  //}
}

void ModelCreator::CreateNewGrowthModule() {
  std::string currentModel = fTreeManager->GetCurrentSelectedModelName();
  // std::string currentElement = TODO: GetCurrentSleectedModelElement
  // Will assume the top-level element folder for now
  Log::Info("Creating Growth Module!");
  fTreeManager->CreateTopLevelElement(M_MODULE_GROWTH);
  fProjectListTree->ClearViewPort();
  fClient->NeedRedraw(fProjectListTree);
}

////////////////////////////////////////////////////////////////////////////////
/// Handle messages send to the ModelCreator object.

Bool_t ModelCreator::ProcessMessage(Long_t msg, Long_t param1, Long_t param2) {
  Char_t strtmp[250];

  switch (GET_MSG(msg)) {
    case kC_COMMAND:

      switch (GET_SUBMSG(msg)) {
        case kCM_BUTTON:
        case kCM_MENU:
          switch (param1) {
            case M_FILE_NEWPROJECT:
              sprintf(strtmp, "Creating New Project");
              fStatusBar->SetText(strtmp, 0);
              new NewProjectDialog(fClient->GetRoot(), this, 800, 400);
              if (fIsNewProject) {
                fIsNewProject = kFALSE;
                CreateNewProject();
              }
              break;

            case M_FILE_OPENPROJECT:
              sprintf(strtmp, "Opening Project");
              fStatusBar->SetText(strtmp, 0);
              break;

            case M_MODEL_NEW:
              new NewModelDialog(fClient->GetRoot(), this, 800, 400);
              if (fIsNewModel) {
                fIsNewModel = kFALSE;
                CreateNewModel();
              }
              sprintf(strtmp, "Creating New Model");
              fStatusBar->SetText(strtmp, 0);
              break;

            case M_MODEL_SIMULATE:
              sprintf(strtmp, "Simulating Model");
              fStatusBar->SetText(strtmp, 0);
              break;

            case M_INTERRUPT_SIMUL:
              Interrupt();
              break;

            case M_FILE_SAVEAS:
              Log::Debug("Clicked save as!");
              break;

            case M_FILE_EXIT:
              CloseWindow();  // this also terminates theApp
              break;

            case M_FILE_PREFERENCES:
              Log::Info("Clicked preferences!");
              break;

            case M_ENTITY_CELL:
              Log::Debug("Clicked cell!");
              CreateNewCell();
              break;

            case M_MODULE_GROWTH:
              Log::Debug("Clicked growth module!");
              CreateNewGrowthModule();
              break;

            case M_MODULE_CHEMOTAXIS:
              Log::Debug("Clicked chemotaxis module!");
              break;

            case M_MODULE_SUBSTANCE:
              Log::Debug("Clicked substance secretion module!");
              break;

            case M_GENERAL_VARIABLE:
              Log::Debug("Clicked general variable!");
              break;

            case M_GENERAL_FUNCTION:
              Log::Debug("Clicked general function!");
              break;

            case M_GENERAL_FORMULA:
              Log::Debug("Clicked general formula!");
              break;
            
            case M_FILE_SAVE:
              Log::Debug("Clicked save!");
              break;

            case M_TOOLS_STARTBROWSER:
              new TBrowser;
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
              fClient->NeedRedraw(fProjectListTree);
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
    fModelFrame->EnableButtons(M_ALL_ACTIVE);
    fModelFrame->SwitchModelTab(selectedModelName.c_str());
  } else {
    fModelFrame->EnableButtons(M_NONE_ACTIVE);
  }
  std::string itemName(fProjectListTree->GetSelected()->GetText());
  if (itemName.find("Cell") != std::string::npos ||
      itemName.find("Growth") != std::string::npos) {
    fModelFrame->ShowModelElement(selectedModelName.c_str(),
                                  itemName.c_str());
    fClient->NeedRedraw(fModelFrame);
    fModelFrame->Resize(10000, 10000);
    fModelFrame->Resize(10001, 10001);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Got close message for this Model Creator. The ModelDislay and the
/// application will be terminated.

void ModelCreator::CloseWindow() {
  std::cout << "Terminating Model Creator" << std::endl;
  DeleteWindow();
  gApplication->Terminate(0);
}

}  // namespace gui