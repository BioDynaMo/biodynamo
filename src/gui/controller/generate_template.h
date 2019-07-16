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


#ifndef GUI_GENERATE_TEMPLATE_H_
#define GUI_GENERATE_TEMPLATE_H_

#include <string>
#include <sstream>
#include "gui/controller/project.h"
#include "core/container/math_array.h"

namespace gui {
  class GenerateTemplate {
   public:
    static GenerateTemplate& GetInstance() {
      static GenerateTemplate instance;  // Instantiated on first use.
      return instance;
    }
    ~GenerateTemplate() = default;
    std::string GenerateCMakeLists(const char* modelName) {
      std::stringstream ss;

      ss << "cmake_minimum_required(VERSION 3.2.0)\n\n"
         << "project(" << modelName << ")\n" 
         << "find_package(BioDynaMo REQUIRED)\n"
         << "include(\"${BDM_USE_FILE}\")\n"
         << "include_directories(\"src\")\n\n"
         << "file(GLOB_RECURSE HEADERS src/*.h)\n"
         << "file(GLOB_RECURSE SOURCES src/*.cc)\n"
         << "bdm_add_executable(" << modelName << "\n"
         << "                    HEADERS \"${HEADERS}\"\n"
         << "                    SOURCES \"${SOURCES}\"\n"
         << "                    LIBRARIES \"${BDM_REQUIRED_LIBRARIES}\")\n";

      std::string output = ss.str();
      return output;
    }
    std::string GenerateSrcH(const char* modelName) {
      std::stringstream ss;
      Model* curModel = Project::GetInstance().GetModel(modelName);
      std::map<std::string, int> elementsMap = curModel->GetModelElements();
      std::map<std::string, int>::iterator it;
      
      ss << "#include \"biodynamo.h\"\n\n"
         << "namespace bdm {\n\n" 
         << "inline int Simulate(int argc, const char** argv) {\n"
         << indent << "Simulation simulation(argc, argv);\n"
         << indent << "auto* rm = simulation.GetResourceManager();\n\n";

      for (it = elementsMap.begin(); it!=elementsMap.end(); ++it) {
        switch(it->second) {
          case M_ENTITY_CELL: {
            ModelElement* elem = curModel->GetModelElement(it->first.c_str());
            std::string cellSrc = GenerateCellSrc(elem);
            ss << cellSrc
               << indent << "rm->push_back(" << elem->GetName() << ");\n\n";
          }
          break;
          default:
           ; // currently only supports cells
        }
      }

      ss << indent << "simulation.GetScheduler()->Simulate(500);\n"
         << indent << "std::cout << \"Simulation completed successfully!\""
         << " << std::endl;\n" << indent << "return 0;\n}\n\n}";

      std::string output = ss.str();
      return output;
    }
    std::string GenerateSrcCC(const char* modelName) {
      std::stringstream ss;

      ss << "#include \"" << modelName << ".h\"\n\n"
         << "int main(int argc, const char** argv) " 
         << "{ return bdm::Simulate(argc, argv); }\n";

      std::string output = ss.str();
      return output;
    }
   private:
     GenerateTemplate() {}
     GenerateTemplate(GenerateTemplate const&) = delete;
     GenerateTemplate& operator=(GenerateTemplate const&) = delete;

     std::string GenerateCellSrc(ModelElement* elem) {
       std::stringstream cellSrc;
       std::string elemName(elem->GetName());
       SimulationEntity* entity = elem->GetEntity();
       bdm::Double3 position(entity->GetPosition());
       bdm::Double3 tractorForce(entity->GetTractorForce());

       cellSrc << indent << "Cell* " << elemName << " = new Cell({" 
               << position[0] << ", " << position[1]
               << ", " << position[2] << "});\n"
               << indent << elemName << "->SetTractorForce({" 
               << tractorForce[0] << ", " << tractorForce[1] 
               << ", " << tractorForce[2] << "});\n";

       std::map<std::string, std::string> attributeMap = elem->GetAttributeMap();
       std::map<std::string, std::string>::iterator it;
       for(it = attributeMap.begin(); it!=attributeMap.end(); ++it) {
         /// Type: double
         if(it->second.find("double") != std::string::npos) { 
           double val = entity->GetAttributeValDouble(it->first.c_str());
           cellSrc << indent << elemName << "->Set" << it->first << "(" << val << ");\n";
         }
         /// Type: unint32_t
         else if(it->second.find("uint32_t") != std::string::npos) {
           uint32_t val = entity->GetAttributeValUInt(it->first.c_str());
           cellSrc << indent << elemName << "->Set" << it->first << "(" << val << ");\n";
         }
         /// other types
         else {
           // Not yet supported
         }
       }
       std::string output = cellSrc.str();
       return output;
     }
     std::string indent = "  ";

  };

}  // namespace gui

#endif  // GUI_GENERATE_TEMPLATE_H_
