// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "core/analysis/style.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
Style::Style() : TNamed(), TAttLine(), TAttFill(), TAttMarker(), TAttText() {
  FromTStyle(gStyle);
}

// -----------------------------------------------------------------------------
Style::~Style() {
  if (tstyle_) {
    delete tstyle_;
  }
}

// -----------------------------------------------------------------------------
TStyle* Style::GetTStyle() const {
  if (!tstyle_) {
    tstyle_ = new TStyle();
    ToTStyle();
  }
  return tstyle_;
}

// -----------------------------------------------------------------------------
Style::operator TStyle*() const { return GetTStyle(); }

// -----------------------------------------------------------------------------
void Style::ToTStyle() const {
  // delete gROOT->GetStyle("Modern");
  auto set_axis = [this](const TAttAxis& axis, Option_t* o) {
    this->tstyle_->SetNdivisions(axis.GetNdivisions(), o);
    this->tstyle_->SetAxisColor(axis.GetAxisColor(), o);
    this->tstyle_->SetLabelColor(axis.GetLabelColor(), o);
    this->tstyle_->SetLabelFont(axis.GetLabelFont(), o);
    this->tstyle_->SetLabelOffset(axis.GetLabelOffset(), o);
    this->tstyle_->SetLabelSize(axis.GetLabelSize(), o);
    this->tstyle_->SetTickLength(axis.GetTickLength(), o);
    this->tstyle_->SetTitleOffset(axis.GetTitleOffset(), o);
    this->tstyle_->SetTitleSize(axis.GetTitleSize(), o);
    this->tstyle_->SetTitleColor(axis.GetTitleColor(), o);
    this->tstyle_->SetTitleFont(axis.GetTitleFont(), o);
  };
  set_axis(fXaxis, "x");
  set_axis(fYaxis, "y");
  set_axis(fZaxis, "z");

  tstyle_->SetBarWidth(fBarWidth);
  tstyle_->SetBarOffset(fBarOffset);
  tstyle_->SetColorModelPS(fColorModelPS);
  tstyle_->SetDrawBorder(fDrawBorder);
  tstyle_->SetOptLogx(fOptLogx);
  tstyle_->SetOptLogy(fOptLogy);
  tstyle_->SetOptLogz(fOptLogz);
  tstyle_->SetOptDate(fOptDate);
  tstyle_->SetOptStat(fOptStat);
  tstyle_->SetOptTitle(fOptTitle);
  tstyle_->SetOptFile(fOptFile);
  tstyle_->SetOptFit(fOptFit);
  tstyle_->SetNumberContours(fNumberContours);
  if (fShowEventStatus != tstyle_->GetShowEventStatus()) {
    tstyle_->ToggleEventStatus();
  }
  if (fShowEditor != tstyle_->GetShowEditor()) {
    tstyle_->ToggleEditor();
  }
  if (fShowToolBar != tstyle_->GetShowToolBar()) {
    tstyle_->ToggleToolBar();
  }
  tstyle_->GetAttDate()->SetTextFont(fAttDate.GetTextFont());
  tstyle_->GetAttDate()->SetTextSize(fAttDate.GetTextSize());
  tstyle_->GetAttDate()->SetTextAngle(fAttDate.GetTextAngle());
  tstyle_->GetAttDate()->SetTextAlign(fAttDate.GetTextAlign());
  tstyle_->GetAttDate()->SetTextColor(fAttDate.GetTextColor());
  tstyle_->SetDateX(fDateX);
  tstyle_->SetDateY(fDateY);
  tstyle_->SetEndErrorSize(fEndErrorSize);
  tstyle_->SetErrorX(fErrorX);
  tstyle_->SetFuncColor(fFuncColor);
  tstyle_->SetFuncStyle(fFuncStyle);
  tstyle_->SetFuncWidth(fFuncWidth);
  tstyle_->SetGridColor(fGridColor);
  tstyle_->SetGridStyle(fGridStyle);
  tstyle_->SetGridWidth(fGridWidth);
  tstyle_->SetLegendBorderSize(fLegendBorderSize);
  tstyle_->SetLegendFillColor(fLegendFillColor);
  tstyle_->SetLegendFont(fLegendFont);
  tstyle_->SetLegendTextSize(fLegendTextSize);
  tstyle_->SetHatchesLineWidth(fHatchesLineWidth);
  tstyle_->SetHatchesSpacing(fHatchesSpacing);
  tstyle_->SetFrameFillColor(fFrameFillColor);
  tstyle_->SetFrameLineColor(fFrameLineColor);
  tstyle_->SetFrameFillStyle(fFrameFillStyle);
  tstyle_->SetFrameLineStyle(fFrameLineStyle);
  tstyle_->SetFrameLineWidth(fFrameLineWidth);
  tstyle_->SetFrameBorderSize(fFrameBorderSize);
  tstyle_->SetFrameBorderMode(fFrameBorderMode);
  tstyle_->SetHistFillColor(fHistFillColor);
  tstyle_->SetHistLineColor(fHistLineColor);
  tstyle_->SetHistFillStyle(fHistFillStyle);
  tstyle_->SetHistLineStyle(fHistLineStyle);
  tstyle_->SetHistLineWidth(fHistLineWidth);
  tstyle_->SetHistMinimumZero(fHistMinimumZero);
  tstyle_->SetHistTopMargin(fHistTopMargin);
  tstyle_->SetCanvasPreferGL(fCanvasPreferGL);
  tstyle_->SetCanvasColor(fCanvasColor);
  tstyle_->SetCanvasBorderSize(fCanvasBorderSize);
  tstyle_->SetCanvasBorderMode(fCanvasBorderMode);
  tstyle_->SetCanvasDefH(fCanvasDefH);
  tstyle_->SetCanvasDefW(fCanvasDefW);
  tstyle_->SetCanvasDefX(fCanvasDefX);
  tstyle_->SetCanvasDefY(fCanvasDefY);
  tstyle_->SetPadColor(fPadColor);
  tstyle_->SetPadBorderSize(fPadBorderSize);
  tstyle_->SetPadBorderMode(fPadBorderMode);
  tstyle_->SetPadBottomMargin(fPadBottomMargin);
  tstyle_->SetPadTopMargin(fPadTopMargin);
  tstyle_->SetPadLeftMargin(fPadLeftMargin);
  tstyle_->SetPadRightMargin(fPadRightMargin);
  tstyle_->SetPadGridX(fPadGridX);
  tstyle_->SetPadGridY(fPadGridY);
  tstyle_->SetPadTickX(fPadTickX);
  tstyle_->SetPadTickY(fPadTickY);
  tstyle_->SetPaperSize(fPaperSizeX, fPaperSizeY);
  tstyle_->SetScreenFactor(fScreenFactor);
  tstyle_->SetStatColor(fStatColor);
  tstyle_->SetStatTextColor(fStatTextColor);
  tstyle_->SetStatBorderSize(fStatBorderSize);
  tstyle_->SetStatFont(fStatFont);
  tstyle_->SetStatFontSize(fStatFontSize);
  tstyle_->SetStatStyle(fStatStyle);
  tstyle_->SetStatFormat(fStatFormat);
  tstyle_->SetStatX(fStatX);
  tstyle_->SetStatY(fStatY);
  tstyle_->SetStatW(fStatW);
  tstyle_->SetStatH(fStatH);
  tstyle_->SetStripDecimals(fStripDecimals);
  tstyle_->SetTitleAlign(fTitleAlign);
  tstyle_->SetTitleColor(fTitleColor);
  tstyle_->SetTitleTextColor(fTitleTextColor);
  tstyle_->SetTitleBorderSize(fTitleBorderSize);
  tstyle_->SetTitleFont(fTitleFont);
  tstyle_->SetTitleFontSize(fTitleFontSize);
  tstyle_->SetTitleStyle(fTitleStyle);
  tstyle_->SetTitleX(fTitleX);
  tstyle_->SetTitleY(fTitleY);
  tstyle_->SetTitleW(fTitleW);
  tstyle_->SetTitleH(fTitleH);
  tstyle_->SetLegoInnerR(fLegoInnerR);

  tstyle_->SetHeaderPS(fHeaderPS);
  tstyle_->SetTitlePS(fTitlePS);
  tstyle_->SetFitFormat(fFitFormat);
  tstyle_->SetPaintTextFormat(fPaintTextFormat);
  tstyle_->SetLineScalePS(fLineScalePS);
  tstyle_->SetJoinLinePS(fJoinLinePS);
  tstyle_->SetCapLinePS(fCapLinePS);
  tstyle_->SetColorModelPS(fColorModelPS);
  tstyle_->SetTimeOffset(fTimeOffset);
  tstyle_->SetImageScaling(fImageScaling);

  tstyle_->SetLineColor(fLineColor);
  tstyle_->SetLineStyle(fLineStyle);
  tstyle_->SetLineWidth(GetLineWidth());
  tstyle_->SetFillColor(GetFillColor());
  tstyle_->SetFillStyle(GetFillStyle());
  tstyle_->SetMarkerColor(GetMarkerColor());
  tstyle_->SetMarkerSize(GetMarkerSize());
  tstyle_->SetMarkerStyle(GetMarkerStyle());
  tstyle_->SetTextAlign(GetTextAlign());
  tstyle_->SetTextAngle(GetTextAngle());
  tstyle_->SetTextColor(GetTextColor());
  tstyle_->SetTextFont(GetTextFont());
  tstyle_->SetTextSize(GetTextSize());
}

// -----------------------------------------------------------------------------
void Style::FromTStyle(TStyle* style) {
  auto set_axis = [style](TAttAxis& axis, Option_t* o) {
    axis.SetNdivisions(style->GetNdivisions(o));
    axis.SetAxisColor(style->GetAxisColor(o));
    axis.SetLabelColor(style->GetLabelColor(o));
    axis.SetLabelFont(style->GetLabelFont(o));
    axis.SetLabelOffset(style->GetLabelOffset(o));
    axis.SetTickLength(style->GetTickLength(o));
    axis.SetTitleOffset(style->GetTitleOffset(o));
    axis.SetTitleSize(style->GetTitleSize(o));
    axis.SetTitleColor(style->GetTitleColor(o));
    axis.SetTitleFont(style->GetTitleFont(o));
  };
  set_axis(fXaxis, "x");
  set_axis(fYaxis, "y");
  set_axis(fZaxis, "z");

  fBarWidth = style->GetBarWidth();
  fBarOffset = style->GetBarOffset();
  fColorModelPS = style->GetColorModelPS();
  fDrawBorder = style->GetDrawBorder();
  fOptLogx = style->GetOptLogx();
  fOptLogy = style->GetOptLogy();
  fOptLogz = style->GetOptLogz();
  fOptDate = style->GetOptDate();
  fOptStat = style->GetOptStat();
  fOptTitle = style->GetOptTitle();
  fOptFile = style->GetOptFile();
  fOptFit = style->GetOptFit();
  fShowEventStatus = style->GetShowEventStatus();
  fShowEditor = style->GetShowEditor();
  fShowToolBar = style->GetShowToolBar();
  fNumberContours = style->GetNumberContours();
  fAttDate.SetTextFont(style->GetAttDate()->GetTextFont());
  fAttDate.SetTextSize(style->GetAttDate()->GetTextSize());
  fAttDate.SetTextAngle(style->GetAttDate()->GetTextAngle());
  fAttDate.SetTextAlign(style->GetAttDate()->GetTextAlign());
  fAttDate.SetTextColor(style->GetAttDate()->GetTextColor());
  fDateX = style->GetDateX();
  fDateY = style->GetDateY();
  fEndErrorSize = style->GetEndErrorSize();
  fErrorX = style->GetErrorX();
  fFuncColor = style->GetFuncColor();
  fFuncStyle = style->GetFuncStyle();
  fFuncWidth = style->GetFuncWidth();
  fGridColor = style->GetGridColor();
  fGridStyle = style->GetGridStyle();
  fGridWidth = style->GetGridWidth();
  fLegendBorderSize = style->GetLegendBorderSize();
  fLegendFillColor = style->GetLegendFillColor();
  fLegendFont = style->GetLegendFont();
  fLegendTextSize = style->GetLegendTextSize();
  fHatchesLineWidth = style->GetHatchesLineWidth();
  fHatchesSpacing = style->GetHatchesSpacing();
  fFrameFillColor = style->GetFrameFillColor();
  fFrameLineColor = style->GetFrameLineColor();
  fFrameFillStyle = style->GetFrameFillStyle();
  fFrameLineStyle = style->GetFrameLineStyle();
  fFrameLineWidth = style->GetFrameLineWidth();
  fFrameBorderSize = style->GetFrameBorderSize();
  fFrameBorderMode = style->GetFrameBorderMode();
  fHistFillColor = style->GetHistFillColor();
  fHistLineColor = style->GetHistLineColor();
  fHistFillStyle = style->GetHistFillStyle();
  fHistLineStyle = style->GetHistLineStyle();
  fHistLineWidth = style->GetHistLineWidth();
  fHistMinimumZero = style->GetHistMinimumZero();
  fHistTopMargin = style->GetHistTopMargin();
  fCanvasPreferGL = style->GetCanvasPreferGL();
  fCanvasColor = style->GetCanvasColor();
  fCanvasBorderSize = style->GetCanvasBorderSize();
  fCanvasBorderMode = style->GetCanvasBorderMode();
  fCanvasDefH = style->GetCanvasDefH();
  fCanvasDefW = style->GetCanvasDefW();
  fCanvasDefX = style->GetCanvasDefX();
  fCanvasDefY = style->GetCanvasDefY();
  fPadColor = style->GetPadColor();
  fPadBorderSize = style->GetPadBorderSize();
  fPadBorderMode = style->GetPadBorderMode();
  fPadBottomMargin = style->GetPadBottomMargin();
  fPadTopMargin = style->GetPadTopMargin();
  fPadLeftMargin = style->GetPadLeftMargin();
  fPadRightMargin = style->GetPadRightMargin();
  fPadGridX = style->GetPadGridX();
  fPadGridY = style->GetPadGridY();
  fPadTickX = style->GetPadTickX();
  fPadTickY = style->GetPadTickY();
  style->GetPaperSize(fPaperSizeX, fPaperSizeY);
  fScreenFactor = style->GetScreenFactor();
  fStatColor = style->GetStatColor();
  fStatTextColor = style->GetStatTextColor();
  fStatBorderSize = style->GetStatBorderSize();
  fStatFont = style->GetStatFont();
  fStatFontSize = style->GetStatFontSize();
  fStatStyle = style->GetStatStyle();
  fStatFormat = style->GetStatFormat();
  fStatX = style->GetStatX();
  fStatY = style->GetStatY();
  fStatW = style->GetStatW();
  fStatH = style->GetStatH();
  fStripDecimals = style->GetStripDecimals();
  fTitleAlign = style->GetTitleAlign();
  fTitleColor = style->GetTitleColor();
  fTitleTextColor = style->GetTitleTextColor();
  fTitleBorderSize = style->GetTitleBorderSize();
  fTitleFont = style->GetTitleFont();
  fTitleFontSize = style->GetTitleFontSize();
  fTitleStyle = style->GetTitleStyle();
  fTitleX = style->GetTitleX();
  fTitleY = style->GetTitleY();
  fTitleW = style->GetTitleW();
  fTitleH = style->GetTitleH();
  fLegoInnerR = style->GetLegoInnerR();

  fHeaderPS = style->GetHeaderPS();
  fTitlePS = style->GetTitlePS();
  fFitFormat = style->GetFitFormat();
  fPaintTextFormat = style->GetPaintTextFormat();
  fLineScalePS = style->GetLineScalePS();
  fJoinLinePS = style->GetJoinLinePS();
  fCapLinePS = style->GetCapLinePS();
  fColorModelPS = style->GetColorModelPS();
  fTimeOffset = style->GetTimeOffset();
  fImageScaling = style->GetImageScaling();

  fLineColor = style->GetLineColor();
  fLineStyle = style->GetLineStyle();
  SetLineWidth(style->GetLineWidth());
  SetFillColor(style->GetFillColor());
  SetFillStyle(style->GetFillStyle());
  SetMarkerColor(style->GetMarkerColor());
  SetMarkerSize(style->GetMarkerSize());
  SetMarkerStyle(style->GetMarkerStyle());
  SetTextAlign(style->GetTextAlign());
  SetTextAngle(style->GetTextAngle());
  SetTextColor(style->GetTextColor());
  SetTextFont(style->GetTextFont());
  SetTextSize(style->GetTextSize());
}

}  // namespace experimental
}  // namespace bdm
