#ifndef CORE_GUI_LOG_H_
#define CORE_GUI_LOG_H_

#include <TGTextEdit.h>
#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include "core/util/log.h"
#include "core/util/string.h"
#include "gui/gui_constants.h"

class GUILog {

 public:
  template <typename... Args>
  static void Info(const Args&... parts) {
    std::string message = bdm::Concat("INFO:", CurrentDateTime(), parts...);
    bdm::Log::Info(LogFile, message, "\n");
    LogMessage(message);
  }

  template <typename... Args>
  static void Debug(const Args&... parts){
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
  static void SetLogFile(std::string location);

 private:
  static TGTextEdit* TEdit;
  static std::string LogFile;
  
  static const std::string CurrentDateTime();
  static void LogMessage(const std::string message);
};


#endif  // CORE_GUI_LOG_H_