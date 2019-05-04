#ifndef CORE_GUI_LOG_H_
#define CORE_GUI_LOG_H_

#include <TGTextEdit.h>
#include <TError.h>
#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include "core/util/log.h"
#include "core/util/string.h"
#include "gui/gui_constants.h"

class GUILog {

 public:
  template <typename... Args>
  static void Info(const Args&... parts) {
    LogMessage(Constants::logFile, Constants::tEdit, "INFO:", CurrentDateTime(), parts...);
  }
  template <typename... Args>
  static void Debug(const Args&... parts) {
    LogMessage(Constants::logFile, Constants::tEdit, "DEBUG:", CurrentDateTime(), parts...);
  }
  template <typename... Args>
  static void Warning(const Args&... parts) {
    LogMessage(Constants::logFile, Constants::tEdit, "WARNING:", CurrentDateTime(), parts...);
  }
  template <typename... Args>
  static void Error(const Args&... parts) {
    LogMessage(Constants::logFile, Constants::tEdit, "ERROR:", CurrentDateTime(), parts...);
  }

 private:
  // Get current date/time, format is YYYY-MM-DD.HH:mm:ss
  static const std::string CurrentDateTime() {
      time_t     now = time(0);
      struct tm  tstruct;
      char       buf[80];
      tstruct = *localtime(&now);
      strftime(buf, sizeof(buf), "%Y-%m-%d.%X: ", &tstruct);
      return buf;
  }

  template <typename... Args>
  static void LogMessage(const std::string logFile, TGTextEdit* tEdit, const Args&... parts) {
    std::string message = bdm::Concat(parts...);

    bdm::Log::Info(logFile, message, "\n");

    if (!logFile.empty()) {
      std::ofstream ofs(logFile, std::ofstream::out | std::ofstream::app);
      ofs << message << "\n";
      ofs.close();
    }
    
    if(tEdit != nullptr) {
      uint msgSize = message.size();
      for(uint i = 0; i < msgSize; i++) {
          tEdit->InsChar(message.at(i));
      }
      tEdit->BreakLine();
    }
  }
};

#endif  // CORE_GUI_LOG_H_