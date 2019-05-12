#ifndef CORE_GUI_LOG_CC_
#define CORE_GUI_LOG_CC_

#include "core/util/log.h"
#include "core/util/string.h"
#include "gui/gui_log.h"

TGTextEdit* GUILog::TEdit = nullptr;
std::string GUILog::LogFile = "";

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string GUILog::CurrentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X: ", &tstruct);
    return buf;
}
//template <typename... Args>
void GUILog::LogMessage(const std::string message) {
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
}
void GUILog::SetTextEdit(TGTextEdit* tEdit) {
  TEdit = tEdit;
}
void GUILog::SetLogFile(std::string location) {
  LogFile = location;
}


#endif  // CORE_GUI_LOG_CC_