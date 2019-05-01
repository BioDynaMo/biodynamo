#ifndef CORE_GUI_H_
#define CORE_GUI_H_

#include <stdlib.h>
#include <TROOT.h>
#include <TClass.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TVirtualPadEditor.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TGSlider.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGTextEdit.h>
#include <TGShutter.h>
#include <TGProgressBar.h>
#include <TGColorSelect.h>
#include <RQ_OBJECT.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>


enum ETestCommandIdentifiers {
   M_FILE_OPEN,
   M_FILE_SAVE,
   M_FILE_SAVEAS,
   M_FILE_PRINT,
   M_FILE_PRINTSETUP,
   M_FILE_EXIT,
   M_TEST_DLG,
   M_TEST_MSGBOX,
   M_TEST_SLIDER,
   M_TEST_SHUTTER,
   M_TEST_DIRLIST,
   M_TEST_FILELIST,
   M_TEST_PROGRESS,
   M_TEST_NUMBERENTRY,
   M_TEST_FONTDIALOG,
   M_TEST_NEWMENU,
   M_VIEW_ENBL_DOCK,
   M_VIEW_ENBL_HIDE,
   M_VIEW_DOCK,
   M_VIEW_UNDOCK,
   M_HELP_CONTENTS,
   M_HELP_SEARCH,
   M_HELP_ABOUT,
   M_CASCADE_1,
   M_CASCADE_2,
   M_CASCADE_3,
   M_NEW_REMOVEMENU,
   VId1,
   HId1,
   VId2,
   HId2,
   VSId1,
   HSId1,
   VSId2,
   HSId2
};

struct shutterData_t {
   const char *pixmap_name;
   const char *tip_text;
   Int_t       id;
   TGButton   *button;
};

class TileFrame;
class TestMainFrame {
RQ_OBJECT("TestMainFrame")
private:
   TGMainFrame        *fMain;
   TGDockableFrame    *fMenuDock;
   TGCompositeFrame   *fStatusFrame;
   TGCanvas           *fCanvasWindow;
   TileFrame          *fContainer;
   TGTextEntry        *fTestText;
   TGButton           *fTestButton;
   TGColorSelect      *fColorSel;
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuTest, *fMenuView, *fMenuHelp;
   TGPopupMenu        *fCascadeMenu, *fCascade1Menu, *fCascade2Menu;
   TGPopupMenu        *fMenuNew1, *fMenuNew2;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;
public:
   TestMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~TestMainFrame();
   // slots
   void CloseWindow();
   void DoButton();
   void HandleMenu(Int_t id);
   void HandlePopup() { printf("menu popped up\n"); }
   void HandlePopdown() { printf("menu popped down\n"); }
   void Created() { Emit("Created()"); } //*SIGNAL*
   void Welcome() { printf("TestMainFrame has been created. Welcome!\n"); }
};
class TestDialog {
RQ_OBJECT("TestDialog")
private:
   TGTransientFrame    *fMain;
   TGCompositeFrame    *fFrame1, *fF1, *fF2, *fF3, *fF4, *fF5;
   TGGroupFrame        *fF6, *fF7;
   TGButton            *fOkButton, *fCancelButton, *fStartB, *fStopB;
   TGButton            *fBtn1, *fBtn2, *fChk1, *fChk2, *fRad1, *fRad2;
   TGPictureButton     *fPicBut1;
   TGCheckButton       *fCheck1;
   TGCheckButton       *fCheckMulti;
   TGListBox           *fListBox;
   TGComboBox          *fCombo;
   TGTab               *fTab;
   TGTextEntry         *fTxt1, *fTxt2;
   TGLayoutHints       *fL1, *fL2, *fL3, *fL4;
   TRootEmbeddedCanvas *fEc1, *fEc2;
   Int_t                fFirstEntry;
   Int_t                fLastEntry;
   Bool_t               fFillHistos;
   TH1F                *fHpx;
   TH2F                *fHpxpy;
   void FillHistos();
public:
   TestDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
               UInt_t options = kVerticalFrame);
   virtual ~TestDialog();
   // slots
   void DoClose();
   void CloseWindow();
   void DoOK();
   void DoCancel();
   void DoTab(Int_t id);
   void HandleButtons(Int_t id = -1);
   void HandleEmbeddedCanvas(Int_t event, Int_t x, Int_t y, TObject *sel);
};
class TestMsgBox {
RQ_OBJECT("TestMsgBox")
private:
   TGTransientFrame     *fMain;
   TGCompositeFrame     *f1, *f2, *f3, *f4, *f5;
   TGButton             *fTestButton, *fCloseButton;
   TGPictureButton      *fPictButton;
   TGRadioButton        *fR[4];
   TGCheckButton        *fC[13];
   TGGroupFrame         *fG1, *fG2;
   TGLayoutHints        *fL1, *fL2, *fL3, *fL4, *fL5, *fL6, *fL21;
   TGTextEntry          *fTitle, *fMsg;
   TGTextBuffer         *fTbtitle, *fTbmsg;
   TGLabel              *fLtitle, *fLmsg;
   TGGC                  fRedTextGC;
public:
   TestMsgBox(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
              UInt_t options = kVerticalFrame);
   virtual ~TestMsgBox();
   // slots
   void TryToClose();
   void CloseWindow();
   void DoClose();
   void DoRadio();
   void DoTest();
};
class TestSliders {
RQ_OBJECT("TestSliders")
private:
   TGTransientFrame  *fMain;
   TGVerticalFrame   *fVframe1, *fVframe2;
   TGLayoutHints     *fBly, *fBfly1;
   TGHSlider         *fHslider1, *fHslider2;
   TGVSlider         *fVslider1;
   TGDoubleVSlider   *fVslider2;
   TGTextEntry       *fTeh1, *fTev1, *fTeh2, *fTev2;
   TGTextBuffer      *fTbh1, *fTbv1, *fTbh2, *fTbv2;
public:
   TestSliders(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
   virtual ~TestSliders();
   // slots
   void CloseWindow();
   void DoText(const char *text);
   void DoSlider(Int_t pos = 0);
};
class TestShutter {
RQ_OBJECT("TestShutter")
private:
   TGTransientFrame *fMain;
   TGShutter        *fShutter;
   TGLayoutHints    *fLayout;
   const TGPicture  *fDefaultPic;
public:
   TestShutter(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
   ~TestShutter();
   void AddShutterItem(const char *name, shutterData_t *data);
   // slots
   void CloseWindow();
   void HandleButtons();
};
class TestDirList {
RQ_OBJECT("TestDirList")
protected:
   TGTransientFrame *fMain;
   TGListTree       *fContents;
   const TGPicture  *fIcon;
   TString DirName(TGListTreeItem* item);
public:
   TestDirList(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
   virtual ~TestDirList();
   // slots
   void OnDoubleClick(TGListTreeItem* item, Int_t btn);
   void CloseWindow();
};
class TestFileList {
RQ_OBJECT("TestFileList")
protected:
   TGTransientFrame *fMain;
   TGFileContainer  *fContents;
   TGPopupMenu      *fMenu;
   void DisplayFile(const TString &fname);
   void DisplayDirectory(const TString &fname);
   void DisplayObject(const TString& fname,const TString& name);
public:
   TestFileList(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
   virtual ~TestFileList();
   // slots
   void OnDoubleClick(TGLVEntry*,Int_t);
   void DoMenu(Int_t);
   void CloseWindow();
};
class TestProgress {
private:
   TGTransientFrame  *fMain;
   TGHorizontalFrame *fHframe1;
   TGVerticalFrame   *fVframe1;
   TGLayoutHints     *fHint1, *fHint2, *fHint3, *fHint4, *fHint5;
   TGHProgressBar    *fHProg1, *fHProg2, *fHProg3;
   TGVProgressBar    *fVProg1, *fVProg2;
   TGTextButton      *fGO;
   Bool_t             fClose;
public:
   TestProgress(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
   virtual ~TestProgress();
   // slots
   void CloseWindow();
   void DoClose();
   void DoGo();
};
class EntryTestDlg {
private:
   TGTransientFrame     *fMain;
   TGVerticalFrame      *fF1;
   TGVerticalFrame      *fF2;
   TGHorizontalFrame    *fF[13];
   TGLayoutHints        *fL1;
   TGLayoutHints        *fL2;
   TGLayoutHints        *fL3;
   TGLabel              *fLabel[13];
   TGNumberEntry        *fNumericEntries[13];
   TGCheckButton        *fLowerLimit;
   TGCheckButton        *fUpperLimit;
   TGNumberEntry        *fLimits[2];
   TGCheckButton        *fPositive;
   TGCheckButton        *fNonNegative;
   TGButton             *fSetButton;
   TGButton             *fExitButton;
//   static const char *const numlabel[13];
//   static const Double_t numinit[13];
public:
   EntryTestDlg(const TGWindow *p, const TGWindow *main);
   virtual ~EntryTestDlg();
   // slots
   void CloseWindow();
   void SetLimits();
   void DoOK();
};
class Editor {
private:
   TGTransientFrame *fMain;   // main frame of this widget
   TGTextEdit       *fEdit;   // text edit widget
   TGTextButton     *fOK;     // OK button
   TGLayoutHints    *fL1;     // layout of TGTextEdit
   TGLayoutHints    *fL2;     // layout of OK button
public:
   Editor(const TGWindow *main, UInt_t w, UInt_t h);
   virtual ~Editor();
   void   LoadFile(const char *file);
   void   LoadBuffer(const char *buffer);
   void   AddBuffer(const char *buffer);
   TGTextEdit *GetEditor() const { return fEdit; }
   void   SetTitle();
   void   Popup();
   // slots
   void   CloseWindow();
   void   DoOK();
   void   DoOpen();
   void   DoSave();
   void   DoClose();
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