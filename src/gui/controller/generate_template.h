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

#include <algorithm>
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
         << "file(GLOB_RECURSE HEADERS \n"
         << "    \"src/*.h\",\n"
         << "    \"$ENV{BDM_SRC_DIR}/core/simulation_backup.h\")\n"
         << "file(GLOB_RECURSE SOURCES src/*.cc)\n"
         << "bdm_add_executable(" << modelName << "\n"
         << "                    HEADERS \"${HEADERS}\"\n"
         << "                    SOURCES \"${SOURCES}\"\n"
         << "                    LIBRARIES \"${BDM_REQUIRED_LIBRARIES}\")\n";

      std::string output = ss.str();
      return output;
    }

    std::string GenerateSrcH(const char* modelName, const char* backupFileName, const char* diffusionFileName) {
      std::stringstream ss;
      Model* curModel = Project::GetInstance().GetModel(modelName);
      std::map<std::string, int> elementsMap = curModel->GetModelElements();
      std::map<std::string, int>::iterator it;
      
      std::string modelNameUpper(modelName);
      std::transform(modelNameUpper.begin(), modelNameUpper.end(), modelNameUpper.begin(), ::toupper);

      ss << "#ifndef " << modelNameUpper << "_H_\n";
      ss << "#define " << modelNameUpper << "_H_\n";
      
      ss << "#include \"biodynamo.h\"\n"
         << "#include \"core/simulation_backup.h\"\n";

      if(fEnableDiffusion && strcmp(diffusionFileName, "") != 0) {
        ss << "#include \"" << diffusionFileName << "\"\n";
      }

      ss << "namespace bdm {\n\n" 
         << "inline int Simulate(int argc, const char** argv) {\n"
         << tab << "Simulation simulation(argc, argv);\n"
         << tab << "auto* rm = simulation.GetResourceManager();\n\n";

      if(fEnableDiffusion) {
        ss << tab << "ModelInitializer::DefineSubstance(kKalium, \"Kalium\", 0.4, 0, 25);\n";
      }

      Bool_t firstCell = kTRUE;
      for (it = elementsMap.begin(); it!=elementsMap.end(); ++it) {
        switch(it->second) {
          case M_ENTITY_CELL: {
            ModelElement* elem = curModel->GetModelElement(it->first.c_str());
            std::string cellSrc;
            // First cell will secrete
            if(fEnableDiffusion && firstCell) {
              cellSrc = GenerateCellSrc(elem, kTRUE);
              firstCell = kFALSE;
            } else {
              cellSrc = GenerateCellSrc(elem, kFALSE);
            }
            ss << cellSrc
               << tab << "rm->push_back(" << elem->GetName() << ");\n\n";
          }
          break;
          default:
           ; // currently only supports cells
        }
      }
      
      ss << tab << "/// Needed for connecting to gui\n"
         << tab << "simulation.GetScheduler()->Simulate(1);\n";
      //if(!fEnableDiffusion) {
      //  ss << tab << "SimulationBackup* simBackup = new SimulationBackup(\"" << backupFileName << "\", \"\");\n"
      //     << tab << "simBackup->Backup(1);\n"
      //     << tab << "delete simBackup;\n\n"
      //     << tab << "simulation.GetScheduler()->Simulate(500);\n";
      //}
      ss << tab << "std::cout << \"Simulation completed successfully!\" << std::endl;\n"
         << tab << "return 0;\n}\n\n}\n\n"
         << "#endif";

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

    std::string GenerateDiffusionModuleSrcH() {
       std::stringstream diffusionSrc;

       diffusionSrc << "#ifndef DIFFUSION_MODULES_H_\n"
                    << "#define DIFFUSION_MODULES_H_\n"
                    << "#include \"biodynamo.h\"\n\n"
                    << "namespace bdm {\n\n"
                    << "enum Substances { kKalium };\n\n"
                    << "struct Chemotaxis : public BaseBiologyModule {\n"
                    << tab << "BDM_STATELESS_BM_HEADER(Chemotaxis, BaseBiologyModule, 1);\n"
                    << " public:\n"
                    << tab << "Chemotaxis() : BaseBiologyModule(gAllEventIds) {}\n\n"
                    << tab << "void Run(SimObject* so) override {\n"
                    << tab << tab << "auto* sim = Simulation::GetActive();\n"
                    << tab << tab << "auto* rm = sim->GetResourceManager();\n"
                    << tab << tab << "static auto* kDg = rm->GetDiffusionGrid(kKalium);\n"
                    << tab << tab << "if (auto* cell = dynamic_cast<Cell*>(so)) {\n"
                    << tab << tab << tab << "const auto& position = so->GetPosition();\n"
                    << tab << tab << tab << "Double3 gradient;\n"
                    << tab << tab << tab << "kDg->GetGradient(position, &gradient);\n"
                    << tab << tab << tab << "gradient *= 0.5;\n"
                    << tab << tab << tab << "cell->UpdatePosition(gradient);\n"
                    << tab << tab << "}\n"
                    << tab << "}\n"
                    << "};\n\n"
                    << "struct KaliumSecretion : public BaseBiologyModule {\n"
                    << tab << "BDM_STATELESS_BM_HEADER(KaliumSecretion, BaseBiologyModule, 1);\n"
                    << " public:\n"
                    << tab << "KaliumSecretion() : BaseBiologyModule() {}\n\n"
                    << tab << "void Run(SimObject* so) override {\n"
                    << tab << tab << "auto* sim = Simulation::GetActive();\n"
                    << tab << tab << "auto* rm = sim->GetResourceManager();\n"
                    << tab << tab << "static auto* kDg = rm->GetDiffusionGrid(kKalium);\n"
                    << tab << tab << "double amount = 4;\n"
                    << tab << tab << "kDg->IncreaseConcentrationBy(so->GetPosition(), amount);\n"
                    << tab << "}\n"
                    << "};\n\n"
                    << "}\n"
                    << "#endif";

       std::string output = diffusionSrc.str();
       return output;
     }

    void EnableDiffusion() {
      fEnableDiffusion = kTRUE;
    }

   private:
     GenerateTemplate() {}
     GenerateTemplate(GenerateTemplate const&) = delete;
     GenerateTemplate& operator=(GenerateTemplate const&) = delete;

     std::string GenerateCellSrc(ModelElement* elem, Bool_t secretion) {
       std::stringstream cellSrc;
       std::string elemName(elem->GetName());
       SimulationEntity* entity = elem->GetEntity();
       bdm::Double3 position(entity->GetPosition());
       bdm::Double3 tractorForce(entity->GetTractorForce());

       cellSrc << tab << "Cell* " << elemName << " = new Cell({" 
               << position[0] << ", " << position[1]
               << ", " << position[2] << "});\n"
               << tab << elemName << "->SetTractorForce({" 
               << tractorForce[0] << ", " << tractorForce[1] 
               << ", " << tractorForce[2] << "});\n";

       std::map<std::string, std::string> attributeMap = elem->GetAttributeMap();
       std::map<std::string, std::string>::iterator it;
       for(it = attributeMap.begin(); it!=attributeMap.end(); ++it) {
         /// Type: double
         if(it->second.find("double") != std::string::npos) { 
           double val = entity->GetAttributeValDouble(it->first.c_str());
           cellSrc << tab << elemName << "->Set" << it->first << "(" << val << ");\n";
         }
         /// Type: unint32_t
         else if(it->second.find("uint32_t") != std::string::npos) {
           uint32_t val = entity->GetAttributeValUInt(it->first.c_str());
           cellSrc << tab << elemName << "->Set" << it->first << "(" << val << ");\n";
         }
         /// other types
         else {
           // Not yet supported
         }
       }
       if(fEnableDiffusion) {
        cellSrc << tab << elemName << "->AddBiologyModule(new Chemotaxis());\n";
        if(secretion) {
          cellSrc << tab << elemName << "->AddBiologyModule(new KaliumSecretion());\n";
        }
       }
       std::string output = cellSrc.str();
       return output;
     }

     std::string tab = "  ";
     Bool_t fEnableDiffusion = kFALSE;
  };

}  // namespace gui

#endif  // GUI_GENERATE_TEMPLATE_H_
