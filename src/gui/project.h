#ifndef PROJECT_H_
#define PROJECT_H_

#include <string>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

#include <TROOT.h>
#include <TClass.h>
#include <TFile.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <RQ_OBJECT.h>

#include "gui/gui_log.h"

class Project {
  public:
    std::string projectName = "";
    std::string testSetting = "";
};

class ProjectUtil {
 public:

  static void LoadProject(const char* location) {
    GUILog::Info("Loading project at: ", location);
    fileName = location;
    ReadProject();
  }

  static void NewProject(const char* location, const char* projectName) {
    GUILog::Info("Creating new project at: ", fileName);
    std::shared_ptr<Project> p = std::make_shared<Project>();
    p->projectName = projectName;
    p->testSetting = "value 1 2 3";
    fileName = location;
    currentProject = p;
    PrintProjectConfig();
    WriteProject();
  }

  static void SaveProject() {
    GUILog::Info("Saving project at: ", fileName);
  }

  static void SaveAsProject(const char* location) {
    std::shared_ptr<Project> p = std::make_shared<Project>();
    p->projectName = currentProject->projectName;
    p->testSetting = currentProject->testSetting;
    fileName = location;
    currentProject = p;
    WriteProject();
  }

  static void SimulateProject(const char* arg) {
    RunCmd(arg);
  }

 private:
  static std::shared_ptr<Project> currentProject;
  static std::string fileName;

  static void ReadProject() {
    //std::shared_ptr<Project> p = std::make_shared<Project>();
    TFile outfile(fileName.c_str());
    Project* p = new Project;
    outfile.GetObject("Project",p);
    outfile.Close();
    std::shared_ptr<Project> sharedP(p);
    currentProject = sharedP;
    PrintProjectConfig();
  }

  static void WriteProject() {
    //TFile *f = new TFile("hsimple.root","UPDATE")
    TFile outfile(fileName.c_str(), "RECREATE");
    Project *raw_foo = currentProject.get();
    outfile.WriteObject(raw_foo,"Project");
    outfile.Close();
  }

  static void PrintProjectConfig() {
    GUILog::Info("Project Details:");
    GUILog::Info("----------------");
    GUILog::Info("Project Name:");
    GUILog::Info("\t", currentProject->projectName);
    GUILog::Info("Test Setting:");
    GUILog::Info("\t", currentProject->testSetting);
    GUILog::Info("----------------");
  }

  static std::string GetProjectDir() {
    if(!fileName.empty()) {
      std::string directory = fileName;
    }
    size_t found;
    std::cout << "Splitting: " << fileName << std::endl;
    found=fileName.find_last_of("/\\");
    return fileName.substr(0,found);
  }

  static void RunCmd(const char* arg) {
    std::string dir = GetProjectDir();
    std::string cmd = "cd " + dir + " && biodynamo " + arg;
    GUILog::Info("Running cmd:", cmd);
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string tmp = buffer.data();
        //GUILog::Info(tmp);
        result += tmp;
    }
  }
};

#endif // PROJECT_H_