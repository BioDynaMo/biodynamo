// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_ANALYSIS_STYLE_H_
#define CORE_ANALYSIS_STYLE_H_

#include "TStyle.h"

namespace bdm {
namespace experimental {

/// This is just a replacement for TStyle until a bug related to JSON serialization is resolved.
class Style : public TNamed, public TAttLine, public TAttFill, public TAttMarker, public TAttText {

 public:
    Style();
    ~Style();
    operator TStyle*() const;
    TStyle* GetTStyle() const;

private:
   mutable TStyle* tstyle_ = nullptr;  //!

   TAttAxis      fXaxis;             ///< X axis attributes
   TAttAxis      fYaxis;             ///< Y axis attributes
   TAttAxis      fZaxis;             ///< Z axis attributes
   Float_t       fBarWidth;          ///< Width of bar for graphs
   Float_t       fBarOffset;         ///< Offset of bar for graphs
   Int_t         fColorModelPS;      ///< PostScript color model: 0 = RGB, 1 = CMYK
   Int_t         fDrawBorder;        ///< Flag to draw border(=1) or not (0)
   Int_t         fOptLogx;           ///< True if log scale in X
   Int_t         fOptLogy;           ///< True if log scale in y
   Int_t         fOptLogz;           ///< True if log scale in z
   Int_t         fOptDate;           ///< True if date option is selected
   Int_t         fOptStat;           ///< True if option Stat is selected
   Int_t         fOptTitle;          ///< True if option Title is selected
   Int_t         fOptFile;           ///< True if option File is selected
   Int_t         fOptFit;            ///< True if option Fit is selected
   Int_t         fShowEventStatus;   ///< Show event status panel
   Int_t         fShowEditor;        ///< Show pad editor
   Int_t         fShowToolBar;       ///< Show toolbar

   Int_t         fNumberContours;    ///< Default number of contours for 2-d plots
   TAttText      fAttDate;           ///< Canvas date attribute
   Float_t       fDateX;             ///< X position of the date in the canvas (in NDC)
   Float_t       fDateY;             ///< Y position of the date in the canvas (in NDC)
   Float_t       fEndErrorSize;      ///< Size of lines at the end of error bars
   Float_t       fErrorX;            ///< Per cent of bin width for errors along X
   Color_t       fFuncColor;         ///< Function color
   Style_t       fFuncStyle;         ///< Function style
   Width_t       fFuncWidth;         ///< Function line width
   Color_t       fGridColor;         ///< Grid line color (if 0 use axis line color)
   Style_t       fGridStyle;         ///< Grid line style
   Width_t       fGridWidth;         ///< Grid line width
   Width_t       fLegendBorderSize;  ///< Legend box border size
   Color_t       fLegendFillColor;   ///< Legend fill color
   Style_t       fLegendFont;        ///< Legend font style
   Double_t      fLegendTextSize;    ///< Legend text size. If 0 the size is computed automatically
   Int_t         fHatchesLineWidth;  ///< Hatches line width for hatch styles > 3100
   Double_t      fHatchesSpacing;    ///< Hatches spacing for hatch styles > 3100
   Color_t       fFrameFillColor;    ///< Pad frame fill color
   Color_t       fFrameLineColor;    ///< Pad frame line color
   Style_t       fFrameFillStyle;    ///< Pad frame fill style
   Style_t       fFrameLineStyle;    ///< Pad frame line style
   Width_t       fFrameLineWidth;    ///< Pad frame line width
   Width_t       fFrameBorderSize;   ///< Pad frame border size
   Int_t         fFrameBorderMode;   ///< Pad frame border mode
   Color_t       fHistFillColor;     ///< Histogram fill color
   Color_t       fHistLineColor;     ///< Histogram line color
   Style_t       fHistFillStyle;     ///< Histogram fill style
   Style_t       fHistLineStyle;     ///< Histogram line style
   Width_t       fHistLineWidth;     ///< Histogram line width
   Bool_t        fHistMinimumZero;   ///< True if default minimum is 0, false if minimum is automatic
   Double_t      fHistTopMargin;     ///< Margin between histogram's top and pad's top
   Bool_t        fCanvasPreferGL;    ///< If true, rendering in canvas is with GL
   Color_t       fCanvasColor;       ///< Canvas color
   Width_t       fCanvasBorderSize;  ///< Canvas border size
   Int_t         fCanvasBorderMode;  ///< Canvas border mode
   Int_t         fCanvasDefH;        ///< Default canvas height
   Int_t         fCanvasDefW;        ///< Default canvas width
   Int_t         fCanvasDefX;        ///< Default canvas top X position
   Int_t         fCanvasDefY;        ///< Default canvas top Y position
   Color_t       fPadColor;          ///< Pad color
   Width_t       fPadBorderSize;     ///< Pad border size
   Int_t         fPadBorderMode;     ///< Pad border mode
   Float_t       fPadBottomMargin;   ///< Pad bottom margin
   Float_t       fPadTopMargin;      ///< Pad top margin
   Float_t       fPadLeftMargin;     ///< Pad left margin
   Float_t       fPadRightMargin;    ///< Pad right margin
   Bool_t        fPadGridX;          ///< True to get the grid along X
   Bool_t        fPadGridY;          ///< True to get the grid along Y
   Int_t         fPadTickX;          ///< True to set special pad ticks along X
   Int_t         fPadTickY;          ///< True to set special pad ticks along Y
   Float_t       fPaperSizeX;        ///< PostScript paper size along X
   Float_t       fPaperSizeY;        ///< PostScript paper size along Y
   Float_t       fScreenFactor;      ///< Multiplication factor for canvas size and position
   Color_t       fStatColor;         ///< Stat fill area color
   Color_t       fStatTextColor;     ///< Stat text color
   Width_t       fStatBorderSize;    ///< Border size of Stats PaveLabel
   Style_t       fStatFont;          ///< Font style of Stats PaveLabel
   Float_t       fStatFontSize;      ///< Font size in pixels for fonts with precision type 3
   Style_t       fStatStyle;         ///< Fill area style of Stats PaveLabel
   TString       fStatFormat;        ///< Printing format for stats
   Float_t       fStatX;             ///< X position of top right corner of stat box
   Float_t       fStatY;             ///< Y position of top right corner of stat box
   Float_t       fStatW;             ///< Width of stat box
   Float_t       fStatH;             ///< Height of stat box
   Bool_t        fStripDecimals;     ///< Strip decimals in axis labels
   Int_t         fTitleAlign;        ///< Title box alignment
   Color_t       fTitleColor;        ///< Title fill area color
   Color_t       fTitleTextColor;    ///< Title text color
   Width_t       fTitleBorderSize;   ///< Border size of Title PavelLabel
   Style_t       fTitleFont;         ///< Font style of Title PaveLabel
   Float_t       fTitleFontSize;     ///< Font size in pixels for fonts with precision type 3
   Style_t       fTitleStyle;        ///< Fill area style of title PaveLabel
   Float_t       fTitleX;            ///< X position of top left corner of title box
   Float_t       fTitleY;            ///< Y position of top left corner of title box
   Float_t       fTitleW;            ///< Width of title box
   Float_t       fTitleH;            ///< Height of title box
   Float_t       fLegoInnerR;        ///< Inner radius for cylindrical legos
   // This is the attribute that causes problems
   // TString       fLineStyle[30];     ///< String describing line style i (for postScript)
   TString       fHeaderPS;          ///< User defined additional Postscript header
   TString       fTitlePS;           ///< User defined Postscript file title
   TString       fFitFormat;         ///< Printing format for fit parameters
   TString       fPaintTextFormat;   ///< Printing format for TH2::PaintText
   Float_t       fLineScalePS;       ///< Line scale factor when drawing lines on Postscript
   Int_t         fJoinLinePS;        ///< Determines the appearance of joining lines on PostScript, PDF and SVG
   Int_t         fCapLinePS;         ///< Determines the appearance of line caps on PostScript, PDF and SVG
   Double_t      fTimeOffset;        ///< Time offset to the beginning of an axis
   Bool_t        fIsReading;         ///<! Set to FALSE when userclass::UseCurrentStyle is called by the style manager
   Float_t       fImageScaling;      ///< Image scaling to produce high definition bitmap images

   void ToTStyle() const;
   void FromTStyle(TStyle* style);

   ClassDefNV(Style, 1);
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_STYLE_H_

