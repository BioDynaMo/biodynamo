#ifndef CORE_GUI_H_
#define CORE_GUI_H_

#include <stdlib.h>
#include <TROOT.h>
#include <TClass.h>
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
#include <TGFileDialog.h>
#include <TRootEmbeddedCanvas.h>
#include <TColor.h>

enum ETestCommandIdentifiers {
   M_FILE_NEW,
   M_FILE_OPEN,
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

class TileFrame;
class TestMainFrame {
RQ_OBJECT("TestMainFrame")
private:
   TGMainFrame        *fMain;
   TGCanvas           *fCanvasWindow;
   TileFrame          *fContainer;
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuSamples, *fMenuSimulation, *fMenuHelp;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout;
public:
   TestMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~TestMainFrame();
   // slots
   void CloseWindow();
   void HandleMenu(Int_t id);
   void HandlePopup() { printf("menu popped up\n"); }
   void HandlePopdown() { printf("menu popped down\n"); }
   void Created() { Emit("Created()"); } //*SIGNAL*
   void Welcome() { printf("TestMainFrame has been created. Welcome!\n"); }
};

class TileFrame {
RQ_OBJECT("TileFrame")
private:
   TGCompositeFrame *fFrame;
   TGCanvas         *fCanvas;
public:
   TileFrame(const TGWindow *p);
   virtual ~TileFrame() { delete fFrame; }
   TGFrame *GetFrame() const { return fFrame; }
   void SetCanvas(TGCanvas *canvas) { fCanvas = canvas; }
   void HandleMouseWheel(Event_t *event);
};

#endif  // CORE_GUI_H_