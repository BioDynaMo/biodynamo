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

#include "gui/view/log.h"

namespace gui {

std::mutex   Log::Mtx;
std::string  Log::LogFile   = "";
TGTextEdit*  Log::TEdit     = nullptr;
TGStatusBar* Log::StatusBar = nullptr;

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string Log::CurrentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X: ", &tstruct);
    return buf;
}

void Log::LogMessage(const std::string message) {
  Mtx.lock();
  if (!LogFile.empty()) {
    std::ofstream ofs(LogFile, std::ofstream::out | std::ofstream::app);
    ofs << message << "\n";
    ofs.close();
  }
  
  if(TEdit != nullptr) {
    //tEdit->Goto(LONG_MAX, LONG_MAX);
    //tEdit->End();
    uint msgSize = message.size();
    for(uint i = 0; i < msgSize; i++) {
        TEdit->InsChar(message.at(i));
    }
    TEdit->BreakLine();
    //TEdit->ScrollCanvas(100, 1);
    //Emit("DoubleClicked()");
  }

  if(StatusBar != nullptr) {
    /// Only display non-debug messages
    if(message.rfind("DEBUG") != 0) {
      StatusBar->SetText(message.c_str(), 0);
    }
  }
  Mtx.unlock();
}
void Log::SetTextEdit(TGTextEdit* tEdit) {
  TEdit = tEdit;
}
void Log::SetLogFile(const std::string location) {
  std::cout << "Setting log file location: " << location << '\n';
  LogFile = location;
}

void Log::SetStatusBar(TGStatusBar* statusBar) {
  StatusBar = statusBar;
}

}  // namespace gui