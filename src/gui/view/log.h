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

#ifndef GUI_LOG_H_
#define GUI_LOG_H_

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include <TGTextEdit.h>
#include <TGStatusBar.h>

#include "core/util/log.h"
#include "core/util/string.h"
#include "gui/constants.h"

namespace gui {

class Log {
 public:
  template <typename... Args>
  static void Info(const Args&... parts) {
    std::string message = bdm::Concat("INFO:", CurrentDateTime(), parts...);
    bdm::Log::Info(LogFile, message, "\n");
    LogMessage(message);
  }

  template <typename... Args>
  static void Debug(const Args&... parts) {
    std::string message = bdm::Concat("DEBUG:", CurrentDateTime(), parts...);
    bdm::Log::Debug(LogFile, message, "\n");
    LogMessage(message);
  }
  template <typename... Args>
  static void Warning(const Args&... parts) {
    std::string message = bdm::Concat("WARNING:", CurrentDateTime(), parts...);
    bdm::Log::Warning(LogFile, message, "\n");
    LogMessage(message);
  }
  template <typename... Args>
  static void Error(const Args&... parts) {
    std::string message = bdm::Concat("Error:", CurrentDateTime(), parts...);
    bdm::Log::Error(LogFile, message, "\n");
    LogMessage(message);
  }

  static void SetTextEdit(TGTextEdit* tEdit);
  static void SetLogFile(const std::string location);
  static void SetStatusBar(TGStatusBar* statusBar);

 private:
  static TGTextEdit* TEdit;
  static std::string LogFile;
  static TGStatusBar* StatusBar;
  static std::mutex Mtx;

  static const std::string CurrentDateTime();
  static void LogMessage(const std::string message);
};

}  // namespace gui

#endif // GUI_LOG_H_