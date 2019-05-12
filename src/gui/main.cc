#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <TApplication.h>
#include "gui/gui.h"

std::shared_ptr<Project> ProjectUtil::currentProject = nullptr;
std::string ProjectUtil::fileName = "";

void Gui()
{
  new TestMainFrame(gClient->GetRoot(), 800, 600);
}

//---- Main program ------------------------------------------------------------
int main(int argc, char **argv)
{
  if(argc != 2) {
   throw std::invalid_argument("Invalid number of args, please supply log file location");
  }
  GUILog::SetLogFile(argv[1]);

  TApplication theApp("App", &argc, argv);
  Gui();
  theApp.Run();
  return 0;
}

