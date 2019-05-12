#ifndef CORE_GUI_H_
#define CORE_GUI_H_

#include <stdlib.h>
#include <future>

#include <TROOT.h>
#include <TClass.h>
#include <TGButton.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <RQ_OBJECT.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TFrame.h>
#include <TGFileDialog.h>
#include <TRootEmbeddedCanvas.h>
#include <TColor.h>
#include <TGSplitter.h>
#include <TGSplitFrame.h>
#include <TSystem.h>
#include <TGLabel.h>
#include <TGTextEdit.h>
#include <TGTextEntry.h>

#include "gui/gui_constants.h"
#include "gui/gui_log.h"
#include "gui/project.h"

enum ETestCommandIdentifiers {
   M_FILE_NEWPROJECT,
   M_FILE_OPENPROJECT,
   M_FILE_SAVE,
   M_FILE_SAVEAS,
   M_FILE_IMPORT,
   M_FILE_EXPORT,
   M_FILE_PREFERENCES,
   M_FILE_EXIT,
   M_SAMPLES_CELLDIVISION,
   M_SAMPLES_DIFFUSION,
   M_SAMPLES_GENEREGULATION,
   M_SAMPLES_SOMACLUSTERING,
   M_SAMPLES_TUMORCONCEPT,
   M_SIMULATION_GENERATE,
   M_SIMULATION_BUILD,
   M_SIMULATION_RUN,
   M_SIMULATION_OPENPARAVIEW,
   M_HELP_USERGUIDE,
   M_HELP_DEVGUIDE,
   M_HELP_ABOUT
};

//class GUILog {};

class TestMainFrame {
RQ_OBJECT("TestMainFrame")
private:
   TGMainFrame        *fMain;
   TGCanvas           *fCanvasWindow;
   TGVerticalFrame    *fVf;
   TGHorizontalFrame  *fH1, *fH2;
   TGCompositeFrame   *fFtop, *fFbottom;
   TGLabel            *fLtop, *fLbottom;
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuSamples, *fMenuSimulation, *fMenuHelp;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout;
   TGTextEdit         *fEdit;
   std::future<void>   fut;
   std::unique_ptr<Project> project;
   bool                isBuilding;
   bool                isRunning;
public:
   TestMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~TestMainFrame();
   // slots
   void CloseWindow();
   void HandleMenu(Int_t id);
   void Created() { Emit("Created()"); } //*SIGNAL*
   void Welcome() { GUILog::Info("TestMainFrame has been created. Welcome!"); }
   void OpenProject();
   void SaveAsProject();
   void SaveProject();
   void BuildProject();
   void RunProject();
   void EnableSaveAndSimulation();
};


class NewProjectDialog {
RQ_OBJECT("NewProjectDialog")
private:
   TGTransientFrame    *fMain;
   TGVerticalFrame     *fVf;
   TGHorizontalFrame   *fH1, *fH2, *fH3;
   TGGroupFrame        *fF6, *fF7;
   TGButton            *fCreateButton, *fCancelButton;
   TGButton            *fBtn1, *fBtn2, *fChk1, *fChk2, *fRad1, *fRad2;
   TGTextEntry         *fTxt1, *fTxt2;
   TGLayoutHints       *fL1, *fL2, *fL3, *fL4;
   TGTextBuffer        *tBuf1, *tBuf2;
   TGLabel             *fLtop, *fLname, *fLlocation, *fLerror;
   TGPictureButton     *fPictButton;
   const TestMainFrame *mFrame;

public:
   NewProjectDialog(const TGWindow *p, const TGWindow *main, const TestMainFrame *mainFrame, UInt_t w, UInt_t h,
               UInt_t options = kVerticalFrame);
   virtual ~NewProjectDialog();
   // slots
   void DoClose();
   void CloseWindow();
   void DoCreate();
   void DoCancel();
   void DoOpen();
};

#endif  // CORE_GUI_H_