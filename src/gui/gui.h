#ifndef CORE_GUI_H_
#define CORE_GUI_H_

#include <TApplication.h>
#include <TEveBrowser.h>
#include <TEveGeoNode.h>
#include <TEveManager.h>
#include <TGeoManager.h>
#include <TSystem.h>

#include <TGDockableFrame.h>
#include <TGMenu.h>
#include <TGMdiDecorFrame.h>
#include <TG3DLine.h>
#include <TGMdiFrame.h>
#include <TGMdiMainFrame.h>
#include <TGMdiMenu.h>
#include <TGColorSelect.h>
#include <TGListBox.h>
#include <TGNumberEntry.h>
#include <TGScrollBar.h>
#include <TGComboBox.h>
#include <TGuiBldHintsEditor.h>
#include <TRootBrowser.h>
#include <TGuiBldNameFrame.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGShutter.h>
#include <TGButtonGroup.h>
#include <TGCommandPlugin.h>
#include <TGCanvas.h>
#include <TGFSContainer.h>
#include <TGFontDialog.h>
#include <TGuiBldEditor.h>
#include <TGTextEdit.h>
#include <TGButton.h>
#include <TGFSComboBox.h>
#include <TGLabel.h>
#include <TGView.h>
#include <TGMsgBox.h>
#include <TRootGuiBuilder.h>
#include <TGFileBrowser.h>
#include <TGTab.h>
#include <TGListView.h>
#include <TGSplitter.h>
#include <TGTextEditor.h>
#include <TRootCanvas.h>
#include <TGStatusBar.h>
#include <TGListTree.h>
#include <TGuiBldGeometryFrame.h>
#include <TRootControlBar.h>
#include <TGToolTip.h>
#include <TGToolBar.h>
#include <TGuiBldDragManager.h>
#include <TGHtmlBrowser.h>
#include "Riostream.h"

#include "core/sim_object/cell.h"

namespace bdm {

/**
 * Singleton class, which creates graphical user interface for biodynamo
 * simulation
 */
class Gui {
 public:
  static Gui &GetInstance() {
    static Gui instance;  // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }

  /**
   * Creates TEveManager window, initializes members
   */
  void Init() {
    app_ = new TApplication("BDMSim", 0, 0);

    TEveManager::Create();

    geom_ = new TGeoManager("Visualization", "BioDynaMo");

    gEve->GetBrowser()->GetMainFrame()->SetWindowName(
        "Biodynamo Visualization");

    Init_Window();

    init_ = true;
  }

  /**
   * Updates GLViewer of TEveManager according to current state of ECM.
   */
  template <typename TSimulation = Simulation<>>
  void Update() {
    if (!init_)
      throw std::runtime_error("Call GUI::getInstance().Init() first!");

    if (is_geometry_closed_)
      throw std::runtime_error(
          "Geometry is already closed! Don't call GUI::Update() after "
          "GUI::CloseGeometry()!");

    // gEve->FullRedraw3D();
    gSystem->ProcessEvents();

    update_ = true;
  }

 private:
  Gui() : init_(false), update_(false) {}
  Gui(Gui const &) = delete;
  Gui &operator=(Gui const &) = delete;

  void Init_Window()
  {
     // main frame
     TGMainFrame *fMainFrame2628 = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
     fMainFrame2628->SetName("fMainFrame2628");
  }  

  /// Geometry manager
  TGeoManager *geom_;
  TApplication* app_ = nullptr;

  // just to ensure that methods were called in correct order
  bool init_;                // true if init_ was called
  bool update_;              // true if update was called
  bool is_geometry_closed_;  // true if geometry is already closed
};

}  // namespace bdm

#endif  // CORE_GUI_H_