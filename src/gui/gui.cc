#ifndef CORE_GUI_C_
#define CORE_GUI_C_

#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <future>

#include "gui/gui.h"
#include "gui/gui_log.h"


const char *filetypes[] = { "ROOT files",   "*.root",
                            "All files",    "*",
                            0,          0 };


TestMainFrame::TestMainFrame(const TGWindow *p, UInt_t w, UInt_t h)
{
  //
  // Create test main frame. A TGMainFrame is a top level window.
  //
  fMain = new TGMainFrame(p, w, h);
  fMain->SetCleanup(kDeepCleanup); // use hierarchical cleaning
  fMain->Connect("CloseWindow()", "TestMainFrame", this, "CloseWindow()");

  //
  // Create menubar and popup menus. 
  //
  fMenuBar = new TGMenuBar(fMain, 800, 20, kHorizontalFrame);
  fMain->AddFrame(fMenuBar, new TGLayoutHints(kLHintsExpandX, 0, 0, 1, 0));
  fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
  fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);

  // Menu - File
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

  // Menu - Samples
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

  // Menu - Simulation
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

  // Menu - Help
  fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("User Guide", M_HELP_USERGUIDE);
  fMenuHelp->AddEntry("Dev Guide", M_HELP_DEVGUIDE);
  fMenuHelp->AddSeparator();
  fMenuHelp->AddEntry("About", M_HELP_ABOUT);

  fMenuHelp->DisableEntry(M_HELP_USERGUIDE);
  fMenuHelp->DisableEntry(M_HELP_DEVGUIDE);
  fMenuHelp->DisableEntry(M_HELP_ABOUT);

  // Menu button messages are handled by the main frame (i.e. "this") HandleMenu() method.
  fMenuFile->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
  fMenuSamples->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
  fMenuSimulation->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
  fMenuHelp->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
  
  fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Samples", fMenuSamples, fMenuBarItemLayout);
  fMenuBar->AddPopup("S&imulation", fMenuSimulation, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarItemLayout);
  
  //
  // Create horizontal splitter
  //
  fVf = new TGVerticalFrame(fMain,800, 600);
  fH1 = new TGHorizontalFrame(fVf,800,500, kFixedHeight);
  fH2 = new TGHorizontalFrame(fVf,800,100);
  fFtop = new TGCompositeFrame(fH1,1,1, kSunkenFrame);
  fFbottom = new TGCompositeFrame(fH2,1,1,kSunkenFrame);
  fLtop = new TGLabel(fFtop,"Top Frame");
  fLbottom = new TGLabel(fFbottom,"Log Output:");

  fEdit = new TGTextEdit(fFbottom, 800, 100);
  GUILog::SetTextEdit(fEdit);

  fFtop->AddFrame(fLtop, new TGLayoutHints(kLHintsLeft |
            kLHintsCenterY,3,0,0,0));
  fFbottom->AddFrame(fLbottom, new TGLayoutHints(kLHintsLeft |
              kLHintsTop,3,0,0,0));
  fFbottom->AddFrame(fEdit, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY ,2,2,2,2));

  fH1->AddFrame(fFtop, new TGLayoutHints(kLHintsTop |
           kLHintsExpandY | kLHintsExpandX,0,0,1,2));
  fH2->AddFrame(fFbottom,new TGLayoutHints(kLHintsTop |
           kLHintsExpandY | kLHintsExpandX,0,0,1,2));
  fH1->Resize(fFtop->GetDefaultWidth(),fH1->GetDefaultHeight()+20);
  fH2->Resize(fFbottom->GetDefaultWidth(),fH2->GetDefaultHeight()+20);
  
  fVf->AddFrame(fH1, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  
  TGHSplitter *hsplitter = new TGHSplitter(fVf);
  hsplitter->SetFrame(fH1,kTRUE);
  fVf->AddFrame(hsplitter,new TGLayoutHints(kLHintsTop |
           kLHintsExpandX));
  fVf->AddFrame(fH2, new TGLayoutHints(kLHintsBottom |
         kLHintsExpandX | kLHintsExpandY));

  

  fMain->AddFrame(fVf, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
                                  0, 0, 2, 2));

  fMain->SetWindowName("BioDynaMo GUI");
  fMain->MapSubwindows();
  // we need to use GetDefault...() to initialize the layout algorithm...
  fMain->Resize();
  fMain->MapWindow();
  //fMain->Print(); // Will cause issues 
  Connect("Created()", "TestMainFrame", this, "Welcome()");
  Created();
}

TestMainFrame::~TestMainFrame()
{
  // Delete all created widgets.
  delete fMenuFile;
  delete fMenuSamples;
  delete fMenuSimulation;
  delete fMenuHelp;
  delete fVf;
  delete fH1;
  delete fH2;
  delete fFtop;
  delete fFbottom;
  delete fLtop;
  delete fLbottom;
  delete fMenuBar;
  delete fMenuBarLayout;
  delete fMenuBarItemLayout;
  delete fEdit;
  delete fMain;
}

void TestMainFrame::CloseWindow()
{
  // Got close message for this MainFrame. Terminates the application.
  GUILog::Info("Closing GUI");
  gApplication->Terminate();
}

void TestMainFrame::OpenProject()
{
  static TString dir(".");
  TGFileInfo fInfo;
  fInfo.fFileTypes = filetypes;
  fInfo.fIniDir   = StrDup(dir);
  GUILog::Debug("fIniDir = ", fInfo.fIniDir);
  new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fInfo);
  if(!fInfo.fFilename) {
    GUILog::Debug( "Empty filename! Not loading project.");
    return;
  }
  GUILog::Info( "Open file: ", fInfo.fFilename, " (dir:", fInfo.fIniDir, ")");
  ProjectUtil::LoadProject(fInfo.fFilename);
  //TTimer::SingleShot(1, "TestMainFrame", this, "EnableSave()");
  EnableSaveAndSimulation();
}

void TestMainFrame::SaveAsProject()
{
  static TString dir(".");
  TGFileInfo fInfo;
  fInfo.fFileTypes = filetypes;
  fInfo.fIniDir   = StrDup(dir);
  GUILog::Debug("fIniDir = ", fInfo.fIniDir);
  new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fInfo);
  if(!fInfo.fFilename) {
    GUILog::Debug( "Empty filename! Not Saving project.");
    return;
  }
  GUILog::Info( "Saving file: ", fInfo.fFilename, " (dir:", fInfo.fIniDir, ")");
  ProjectUtil::SaveAsProject(fInfo.fFilename);
}

void TestMainFrame::EnableSaveAndSimulation() 
{
  fMenuFile->EnableEntry(M_FILE_SAVE);
  fMenuFile->EnableEntry(M_FILE_SAVEAS);
  fMenuSimulation->EnableEntry(M_SIMULATION_BUILD);
  fMenuSimulation->EnableEntry(M_SIMULATION_RUN);
}

void TestMainFrame::SaveProject()
{  
  ProjectUtil::SaveProject();
}

void RunIt() {
  GUILog::Info("Will run project");

  ProjectUtil::SimulateProject("run");
  
  GUILog::Info("Finished running");
}

void BuildIt() {
  GUILog::Info("Will build project");

  ProjectUtil::SimulateProject("build");
  
  GUILog::Info("Finished building");
}

void TestMainFrame::BuildProject()
{
  fut = std::async(std::launch::async, BuildIt);
}

void TestMainFrame::RunProject()
{
  fut = std::async(std::launch::async, RunIt);
}

void TestMainFrame::HandleMenu(Int_t id)
{
  // Handle menu items.
  switch (id) {
    case M_FILE_NEWPROJECT:
      //TODO: Check if some project is already open
      new NewProjectDialog(gClient->GetRoot(), fMain, this, 400, 200);
      break;      
    case M_FILE_OPENPROJECT:
      OpenProject();
      break;
    case M_FILE_SAVE:
      SaveProject();
      break;
    case M_FILE_SAVEAS:
      SaveAsProject();
      break;
    case M_FILE_IMPORT:
      GUILog::Debug("M_FILE_IMPORT");
      break;
    case M_FILE_EXPORT:
      GUILog::Debug("M_FILE_EXPORT");
      break;
    case M_FILE_PREFERENCES:
      GUILog::Debug("M_FILE_PREFERENCES");
      break;
    case M_FILE_EXIT:
      CloseWindow();  // terminate theApp no need to use SendCloseMessage()
      break;
    case M_SAMPLES_CELLDIVISION:
      GUILog::Debug("M_SAMPLES_CELLDIVISION");
      break;
    case M_SAMPLES_DIFFUSION:
      GUILog::Debug("M_SAMPLES_DIFFUSION");
      break;
    case M_SAMPLES_GENEREGULATION:
      GUILog::Debug("M_SAMPLES_GENEREGULATION");
      break;
    case M_SAMPLES_SOMACLUSTERING:
      GUILog::Debug("M_SAMPLES_SOMACLUSTERING");
      break;
    case M_SAMPLES_TUMORCONCEPT:
      GUILog::Debug("M_SAMPLES_TUMORCONCEPT");
      break;
    case M_SIMULATION_GENERATE:
      GUILog::Debug("M_SIMULATION_GENERATE");
      break;
    case M_SIMULATION_BUILD:
      BuildProject();
      break;
    case M_SIMULATION_RUN:
      RunProject();
      break;
    case M_SIMULATION_OPENPARAVIEW:
      GUILog::Debug("M_SIMULATION_OPENPARAVIEW");
      break;
    case M_HELP_USERGUIDE:
      GUILog::Debug("M_HELP_USERGUIDE");
      break;
    case M_HELP_DEVGUIDE:
      GUILog::Debug("M_HELP_DEVGUIDE");
      break;
    case M_HELP_ABOUT:
      GUILog::Debug("M_HELP_ABOUT");
      break;
    default:
      GUILog::Debug("Menu item %d selected", id);
      break;
  }
}

NewProjectDialog::NewProjectDialog(const TGWindow *p, const TGWindow *main, const TestMainFrame *mainFrame, UInt_t w,
                UInt_t h, UInt_t options)
{
  mFrame = mainFrame;
  // Create a dialog window. A dialog window pops up with respect to its
  // "main" window.
  fMain = new TGTransientFrame(p, main, w, h, options);
  fMain->Connect("CloseWindow()", "NewProjectDialog", this, "DoClose()");
  fMain->DontCallClose(); // to avoid double deletions.
  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);
  
  // Top label
  fVf = new TGVerticalFrame(fMain, 400, 200, kFixedWidth);
  fLtop = new TGLabel(fVf,"Please specify the following to create a new project:");
  fVf->AddFrame(fLtop, new TGLayoutHints(kLHintsTop | kLHintsCenterY | kLHintsCenterX, 0, 0, 0, 0 ));
  
  // Project name
  fH1 = new TGHorizontalFrame(fVf, 400, 60, kFixedWidth);
  fLname = new TGLabel(fH1,"Name:");
  fLerror = new TGLabel(fH1,"Error");
  fLerror->SetTextColor(TColor::RGB2Pixel(255,0,0));
  fLerror->SetText("          ");
  tBuf1 = new TGTextBuffer(80);
  tBuf1->AddText(0, "Untitled");
  fTxt1 = new TGTextEntry(fH1, tBuf1);
  fTxt1->Resize(200, fTxt1->GetDefaultHeight());
  fH1->AddFrame(fLname, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5 ));
  fH1->AddFrame(fTxt1, new TGLayoutHints(kLHintsTop | kLHintsCenterY, 19, 5, 5, 5 ));
  fH1->AddFrame(fLerror, new TGLayoutHints(kLHintsTop | kLHintsCenterY, 5, 5, 5, 5 ));
  fVf->AddFrame(fH1, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 0, 0, 0, 0 ));

  // Project location
  fH2 = new TGHorizontalFrame(fVf, 400, 60, kFixedWidth);
  fLlocation = new TGLabel(fH2,"Location:");
  tBuf2 = new TGTextBuffer(600);
  tBuf2->AddText(0, "/");
  fTxt2 = new TGTextEntry(fH2, tBuf2);
  fTxt2->Resize(200, fTxt2->GetDefaultHeight());
  fPictButton = new TGPictureButton(fH2, gClient->GetPicture("fileopen.xpm"));
  fPictButton->Connect("Clicked()", "NewProjectDialog", this, "DoOpen()");
  fH2->AddFrame(fLlocation, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5 ));
  fH2->AddFrame(fTxt2, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5 ));
  fH2->AddFrame(fPictButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 0, 0, 0 ));
  fVf->AddFrame(fH2, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 0, 0, 0, 0 ));
  
  // Create Create and Cancel buttons in horizontal frame
  fH3 = new TGHorizontalFrame(fVf, 400, 60, kFixedWidth);
  fCreateButton = new TGTextButton(fH3, "&Create", 1);
  fCreateButton->Connect("Clicked()", "NewProjectDialog", this, "DoCreate()");
  fCancelButton = new TGTextButton(fH3, "&Cancel", 2);
  fCancelButton->Connect("Clicked()", "NewProjectDialog", this, "DoCancel()");
  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
                  2, 2, 2, 2);
  fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);
  fH3->AddFrame(fCreateButton, fL1);
  fH3->AddFrame(fCancelButton, fL1);
  fH3->Resize(150, fCreateButton->GetDefaultHeight());
  fVf->AddFrame(fH3, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 0, 0, 0, 0 ));
  
  fMain->AddFrame(fVf, fL1);
  fMain->MapSubwindows();
  fMain->Resize();
  // position relative to the parent's window
  fMain->CenterOnParent();
  fMain->SetWindowName("Dialog");
  fMain->MapWindow();
  //gClient->WaitFor(fMain);   // otherwise canvas contextmenu does not work
}
NewProjectDialog::~NewProjectDialog()
{
  // Delete test dialog widgets.
  fMain->DeleteWindow();  // deletes fMain
}

void NewProjectDialog::DoClose()
{
  GUILog::Debug("Terminating dialog: via window manager");
  CloseWindow();
}

int EndsWith(const char *str, const char *suffix)
{
   if (!str || !suffix)
      return 0;
   size_t lenstr = strlen(str);
   size_t lensuffix = strlen(suffix);
   if (lensuffix >  lenstr)
      return 0;
   return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void NewProjectDialog::DoOpen()
{
  TString dir(".");
  TGFileInfo fInfo;
  fInfo.fIniDir   = StrDup(dir);
  const char* projectName = fTxt1->GetText();
  if(projectName) {
   fInfo.fFilename  = new char[1000];
   // Will deallocate when fInfo goes out of scope
   strcpy(fInfo.fFilename, projectName);
  } else {
    //TODO: ERROR
  }
  
  fInfo.fFileTypes = filetypes;
  GUILog::Debug("fIniDir = ", fInfo.fIniDir);
  
  new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fInfo);

  if (fInfo.fFilename) {
   const char* suffix = ".root";
   if(!EndsWith(fInfo.fFilename, suffix)) {
    fInfo.fFilename = strcat(fInfo.fFilename, suffix);
   }

   GUILog::Info("Open file: ", fInfo.fFilename, " (dir:", fInfo.fIniDir, ")");
   
   fTxt2->Clear();
   fTxt2->AppendText(fInfo.fFilename);

   char* ptr = strrchr(fInfo.fFilename, '/') + 1;
   ptr[strlen(ptr) - strlen(".root")] = '\0';
   fTxt1->Clear();
   fTxt1->AppendText(ptr);
  }
}

void NewProjectDialog::CloseWindow()
{
  // Called when window is closed via the window manager.
  delete this;
}
void NewProjectDialog::DoCreate()
{
  const char* projectName = fTxt1->GetText();
  const char* projectLocation = fTxt2->GetText();
  GUILog::Debug("Checking for valid project name");

  // TODO: Add more rigourous name checking
  if(!projectName) {
    fLerror->ChangeText("Empty project name!");
    fLerror->Resize();
    return;
  } else if(!projectLocation) {
    fLerror->ChangeText("Empty project location!");
    fLerror->Resize();
    return;
  }

  GUILog::Debug("Terminating dialog: Create pressed");
  // Add protection against double-clicks
  fCreateButton->SetState(kButtonDisabled);
  fCancelButton->SetState(kButtonDisabled);

  // TODO: Display where project will be created?
  ProjectUtil::NewProject(fTxt2->GetText(), projectName);
  Emit("Created()");
  // Send a close message to the main frame. This will trigger the
  // emission of a CloseWindow() signal, which will then call
  // TestDialog::CloseWindow(). Calling directly CloseWindow() will cause
  // a segv since the OK button is still accessed after the DoOK() method.
  // This works since the close message is handled synchronous (via
  // message going to/from X server).
  //fMain->SendCloseMessage();
  // The same effect can be obtained by using a singleshot timer:
  TTimer::SingleShot(50, "TestMainFrame", const_cast<TestMainFrame*>(mFrame), "EnableSaveAndSimulation()");
  TTimer::SingleShot(150, "NewProjectDialog", this, "CloseWindow()");
}

void NewProjectDialog::DoCancel()
{
  GUILog::Debug("Terminating dialog: Cancel pressed");
  // Add protection against double-clicks
  fCreateButton->SetState(kButtonDisabled);
  fCancelButton->SetState(kButtonDisabled);
  TTimer::SingleShot(150, "NewProjectDialog", this, "CloseWindow()");
}

#endif  // CORE_GUI_C_