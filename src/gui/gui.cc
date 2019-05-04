#ifndef CORE_GUI_C_
#define CORE_GUI_C_

#include <stdlib.h>
#include <string>
#include <stdexcept>
#include "gui/gui.h"

const char *filetypes[] = { "All files",     "*",
                            "ROOT files",    "*.root",
                            "ROOT macros",   "*.C",
                            "Text files",    "*.[tT][xX][tT]",
                            0,               0 };

TileFrame::TileFrame(const TGWindow *p)
{
  // Create tile view container. Used to show colormap.
  fFrame = new TGCompositeFrame(p, 10, 10, kHorizontalFrame,
                                TGFrame::GetWhitePixel());
  fFrame->Connect("ProcessedEvent(Event_t*)", "TileFrame", this,
                  "HandleMouseWheel(Event_t*)");
  fCanvas = 0;
  fFrame->SetLayoutManager(new TGTileLayout(fFrame, 8));
  gVirtualX->GrabButton(fFrame->GetId(), kAnyButton, kAnyModifier,
                        kButtonPressMask | kButtonReleaseMask |
                        kPointerMotionMask, kNone, kNone);
}

void TileFrame::HandleMouseWheel(Event_t *event)
{
  // Handle mouse wheel to scroll.
  if (event->fType != kButtonPress && event->fType != kButtonRelease)
     return;
  Int_t page = 0;
  if (event->fCode == kButton4 || event->fCode == kButton5) {
     if (!fCanvas) return;
     if (fCanvas->GetContainer()->GetHeight())
        page = Int_t(Float_t(fCanvas->GetViewPort()->GetHeight() *
                             fCanvas->GetViewPort()->GetHeight()) /
                             fCanvas->GetContainer()->GetHeight());
  }
  if (event->fCode == kButton4) {
     //scroll up
     Int_t newpos = fCanvas->GetVsbPosition() - page;
     if (newpos < 0) newpos = 0;
     fCanvas->SetVsbPosition(newpos);
  }
  if (event->fCode == kButton5) {
     // scroll down
     Int_t newpos = fCanvas->GetVsbPosition() + page;
     fCanvas->SetVsbPosition(newpos);
  }
}

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
  fMenuFile->AddEntry("&New", M_FILE_NEW);
  fMenuFile->AddEntry("&Open...", M_FILE_OPEN);
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
 
  // Keep this for now, could be of later use...
  fMenuFile->Connect("PoppedUp()", "TestMainFrame", this, "HandlePopup()");
  fMenuFile->Connect("PoppedDown()", "TestMainFrame", this, "HandlePopdown()");

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
  Constants::tEdit = fEdit;

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
  //fMain->Print();
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
  delete fContainer;
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
  gApplication->Terminate();
}

void TestMainFrame::HandleMenu(Int_t id)
{
  // Handle menu items.
  switch (id) {
     case M_FILE_OPEN:
        {
           static TString dir(".");
           TGFileInfo fi;
           fi.fFileTypes = filetypes;
           fi.fIniDir    = StrDup(dir);
           GUILog::Info("fIniDir = ", fi.fIniDir);
           new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);
           GUILog::Info( "Open file: ", fi.fFilename, " (dir:", fi.fIniDir, ")");
           dir = fi.fIniDir;
        }
        break;
     case M_FILE_SAVE:
        GUILog::Info("M_FILE_SAVE");
        break;
     case M_FILE_SAVEAS:
        GUILog::Info("M_FILE_SAVEAS");
        break;
     case M_FILE_IMPORT:
        GUILog::Info("M_FILE_IMPORT");
        break;
     case M_FILE_EXPORT:
        GUILog::Info("M_FILE_EXPORT");
        break;
     case M_FILE_PREFERENCES:
        GUILog::Info("M_FILE_PREFERENCES");
        break;
     case M_FILE_EXIT:
        CloseWindow();   // terminate theApp no need to use SendCloseMessage()
        break;
     case M_SAMPLES_CELLDIVISION:
        GUILog::Info("M_SAMPLES_CELLDIVISION");
        break;
     case M_SAMPLES_DIFFUSION:
        GUILog::Info("M_SAMPLES_DIFFUSION");
        break;
     case M_SAMPLES_GENEREGULATION:
        GUILog::Info("M_SAMPLES_GENEREGULATION");
        break;
     case M_SAMPLES_SOMACLUSTERING:
        GUILog::Info("M_SAMPLES_SOMACLUSTERING");
        break;
     case M_SAMPLES_TUMORCONCEPT:
        GUILog::Info("M_SAMPLES_TUMORCONCEPT");
        break;
     case M_SIMULATION_GENERATE:
        GUILog::Info("M_SIMULATION_GENERATE");
        break;
     case M_SIMULATION_BUILD:
        GUILog::Info("M_SIMULATION_BUILD");
        break;
     case M_SIMULATION_RUN:
        GUILog::Info("M_SIMULATION_RUN");
        break;
     case M_SIMULATION_OPENPARAVIEW:
        GUILog::Info("M_SIMULATION_OPENPARAVIEW");
        break;
     case M_HELP_USERGUIDE:
        GUILog::Info("M_HELP_USERGUIDE");
        break;
     case M_HELP_DEVGUIDE:
        GUILog::Info("M_HELP_DEVGUIDE");
        break;
     case M_HELP_ABOUT:
        GUILog::Info("M_HELP_ABOUT");
        break;
     
     default:
        GUILog::Info("Menu item %d selected", id);
        break;
  }
}

void gui()
{
  new TestMainFrame(gClient->GetRoot(), 800, 600);
}

std::string Constants::logFile = "";
TGTextEdit* Constants::tEdit = nullptr;

//---- Main program ------------------------------------------------------------
int main(int argc, char **argv)
{
  if(argc != 2) {
    throw std::invalid_argument("Invalid number of args, please supply log file location");
  }
  Constants::logFile = argv[1];

  //std::string fileName = "/home/luke/Desktop/gui.log";
  //Constants::logFile = fileName;
  TApplication theApp("App", &argc, argv);
  if (gROOT->IsBatch()) {
     fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
     return 1;
  }
  gui();
  theApp.Run();
  return 0;
}

#endif  // CORE_GUI_C_