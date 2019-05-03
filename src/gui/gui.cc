#ifndef CORE_GUI_C_
#define CORE_GUI_C_

#include <stdlib.h>
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
   // Create test main frame. A TGMainFrame is a top level window.
   fMain = new TGMainFrame(p, w, h);
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);
   fMain->Connect("CloseWindow()", "TestMainFrame", this, "CloseWindow()");
   // Create menubar and popup menus. The hint objects are used to place
   // and group the different menu widgets with respect to eachother.
   fMenuBar = new TGMenuBar(fMain, 100, 20, kHorizontalFrame);
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

   // Menu button messages are handled by the main frame (i.e. "this")
   // HandleMenu() method.
   fMenuFile->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
   fMenuSamples->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
   fMenuSimulation->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
   fMenuHelp->Connect("Activated(Int_t)", "TestMainFrame", this, "HandleMenu(Int_t)");
   
   fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Samples", fMenuSamples, fMenuBarItemLayout);
   fMenuBar->AddPopup("S&imulation", fMenuSimulation, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarItemLayout);

   // Create TGCanvas and a canvas container which uses a tile layout manager
   fCanvasWindow = new TGCanvas(fMain, 400, 240);
   fContainer = new TileFrame(fCanvasWindow->GetViewPort());
   fContainer->SetCanvas(fCanvasWindow);
   fCanvasWindow->SetContainer(fContainer->GetFrame());
   // use hierarchical cleaning for container
   fContainer->GetFrame()->SetCleanup(kDeepCleanup);
   // Fill canvas with 256 colored frames
   for (int i=0; i < 256; ++i)
      fCanvasWindow->AddFrame(new TGFrame(fCanvasWindow->GetContainer(),
                              32, 32, 0, TColor::RGB2Pixel(0,0,(i+1)&255)),
                              new TGLayoutHints(kLHintsExpandY | kLHintsRight));
   fMain->AddFrame(fCanvasWindow, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
                                                    0, 0, 2, 2));
   fMain->SetWindowName("BioDynaMo GUI");
   fMain->MapSubwindows();
   // we need to use GetDefault...() to initialize the layout algorithm...
   fMain->Resize();
   fMain->MapWindow();
   fMain->Print();
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
            printf("fIniDir = %s\n", fi.fIniDir);
            new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);
            printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
            dir = fi.fIniDir;
         }
         break;
      case M_FILE_SAVE:
         printf("M_FILE_SAVE\n");
         break;
      case M_FILE_SAVEAS:
         printf("M_FILE_SAVEAS\n");
         break;
      case M_FILE_IMPORT:
         printf("M_FILE_IMPORT\n");
         break;
      case M_FILE_EXPORT:
         printf("M_FILE_EXPORT\n");
         break;
      case M_FILE_PREFERENCES:
         printf("M_FILE_PREFERENCES\n");
         break;
      case M_FILE_EXIT:
         CloseWindow();   // terminate theApp no need to use SendCloseMessage()
         break;
      case M_SAMPLES_CELLDIVISION:
         printf("M_SAMPLES_CELLDIVISION\n");
         break;
      case M_SAMPLES_DIFFUSION:
         printf("M_SAMPLES_DIFFUSION\n");
         break;
      case M_SAMPLES_GENEREGULATION:
         printf("M_SAMPLES_GENEREGULATION\n");
         break;
      case M_SAMPLES_SOMACLUSTERING:
         printf("M_SAMPLES_SOMACLUSTERING\n");
         break;
      case M_SAMPLES_TUMORCONCEPT:
         printf("M_SAMPLES_TUMORCONCEPT\n");
         break;
      case M_SIMULATION_GENERATE:
         printf("M_SIMULATION_GENERATE\n");
         break;
      case M_SIMULATION_BUILD:
         printf("M_SIMULATION_BUILD\n");
         break;
      case M_SIMULATION_RUN:
         printf("M_SIMULATION_RUN\n");
         break;
      case M_SIMULATION_OPENPARAVIEW:
         printf("M_SIMULATION_OPENPARAVIEW\n");
         break;
      case M_HELP_USERGUIDE:
         printf("M_HELP_USERGUIDE\n");
         break;
      case M_HELP_DEVGUIDE:
         printf("M_HELP_DEVGUIDE\n");
         break;
      case M_HELP_ABOUT:
         printf("M_HELP_ABOUT\n");
         break;
      
      default:
         printf("Menu item %d selected\n", id);
         break;
   }
}

void gui()
{
   new TestMainFrame(gClient->GetRoot(), 400, 220);
}

//---- Main program ------------------------------------------------------------
int main(int argc, char **argv)
{
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