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

#ifndef CORE_VISUALIZATION_INSITU_PIPELINE_H_
#define CORE_VISUALIZATION_INSITU_PIPELINE_H_

#include <cstdlib>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/util/log.h"

#if defined(USE_CATALYST) && !defined(__ROOTCLING__)
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPPipeline.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
#include <vtkLiveInsituLink.h>
#include <vtkNew.h>
#include <vtkPVTrivialProducer.h>
#include <vtkSMInputProperty.h>
#include <vtkSMParaViewPipelineControllerWithRendering.h>
#include <vtkSMPluginManager.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>
#include <vtkSMProxyListDomain.h>
#include <vtkSMProxyManager.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMSourceProxy.h>
#include <vtkUnstructuredGrid.h>

#include "core/visualization/catalyst_helper_structs.h"

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

namespace bdm {

#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

class InSituPipeline : public vtkCPPipeline {
 public:
  vtkTypeMacro(InSituPipeline, vtkCPPipeline);
  InSituPipeline() {
    this->pipeline_created_ = false;

    // Get current proxy manager
    proxy_manager_ = vtkSMProxyManager::GetProxyManager();
    session_manager_ = proxy_manager_->GetActiveSessionProxyManager();
    controller_ = vtkSMParaViewPipelineControllerWithRendering::New();
    plugin_manager_ = vtkSMPluginManager::New();
#ifdef __APPLE__
    std::string plugin_path =
        std::string(std::getenv("BDM_INSTALL_DIR")) +
        "/biodynamo/lib/pv_plugin/libBDMGlyphFilter.dylib";
#else
    std::string plugin_path = std::string(std::getenv("BDM_INSTALL_DIR")) +
                              "/biodynamo/lib/pv_plugin/libBDMGlyphFilter.so";
#endif
    // Load custom plugin to enable cylinder glyph scaling
    if (!plugin_manager_->LoadLocalPlugin(plugin_path.c_str())) {
      Fatal("LoadLocalPlugin",
            "Was unable to load our custom visualzation plugin. Have you "
            "sourced the BioDynaMo environment?");
    }
  }

  virtual ~InSituPipeline() {
    // Do not delete proxy_manager_, session_manager_
    plugin_manager_->Delete();
    controller_->Delete();
  }

  void Initialize(
      const std::unordered_map<std::string, VtkSoGrid*>& vtk_so_grids) {
    vtk_so_grids_ = vtk_so_grids;
    initialized_ = true;
  }

  int RequestDataDescription(vtkCPDataDescription* data_description) override {
    if (!data_description) {
      vtkWarningMacro(
          "The data description is empty. There is nothing to visualize!");
      return 0;
    }

    int n_inputs = data_description->GetNumberOfInputDescriptions();

    for (int i = 0; i < n_inputs; i++) {
      data_description->GetInputDescription(i)->AllFieldsOn();
      data_description->GetInputDescription(i)->GenerateMeshOn();
    }

    return 1;
  }

  vtkSMProxy* GetSource(vtkSMProxy* glyph, std::string source_name) {
    vtkSMProxyListDomain* pld = vtkSMProxyListDomain::SafeDownCast(
        glyph->GetProperty("Source")->FindDomain("vtkSMProxyListDomain"));

    for (unsigned int cc = 0; cc < pld->GetNumberOfProxies(); cc++) {
      if (pld->GetProxyName(cc) &&
          strcmp(pld->GetProxyName(cc), source_name.c_str()) == 0) {
        return pld->GetProxy(cc);
      }
    }
    return nullptr;
  }

  // FOR ALL VTK OBJECTS:
  // 1. Get vtkObject + it's unique name
  // 2. Create a producer for it
  // 3. Create a proxy for the producer and register it to the session_manager
  // 4. Create a "real_producer" that sets the vtkObject to its output
  // 5. Add it to the map of producers
  void CreatePipeline(vtkCPDataDescription* data_description) {
    auto n_inputs = data_description->GetNumberOfInputDescriptions();
    for (size_t i = 0; i < n_inputs; i++) {
      auto vtk_object = data_description->GetInputDescription(i)->GetGrid();
      auto object_name = data_description->GetInputDescriptionName(i);

      // Create a vtkPVTrivialProducer and set its output to be the input grid.
      vtkSmartPointer<vtkSMSourceProxy> producer;
      producer.TakeReference(vtkSMSourceProxy::SafeDownCast(
          session_manager_->NewProxy("sources", "PVTrivialProducer")));
      session_manager_->RegisterProxy("sources", object_name, producer);
      producer->UpdateVTKObjects();
      vtkObjectBase* client_side_object = producer->GetClientSideObject();
      vtkPVTrivialProducer* real_producer =
          vtkPVTrivialProducer::SafeDownCast(client_side_object);

      std::string source_name = "SphereSource";
      std::string glyph_type = "Glyph";
      int scale_mode = 0;

      if (vtk_object->IsA("vtkUnstructuredGrid")) {
        real_producer->SetOutput(vtk_object);

        auto& shape = vtk_so_grids_[object_name]->shape_;

        if (shape == Shape::kCylinder) {
          source_name = "CylinderSource";
          glyph_type = "BDMGlyph";
          scale_mode = 4;
        } else if (shape != Shape::kSphere) {
          Log::Warning("CreatePipeline",
                       "We currently support only spherical and cylindrical "
                       "shaped objects for visualization. Received value %d",
                       shape);
        }

        // Create a Glyph filter
        vtkSmartPointer<vtkSMSourceProxy> glyph;
        glyph.TakeReference(vtkSMSourceProxy::SafeDownCast(
            session_manager_->NewProxy("filters", glyph_type.c_str())));
        controller_->PreInitializeProxy(glyph);
        std::string object_name_str = object_name;
        std::string glyph_name = object_name_str + "_Glyph";

        vtkSMPropertyHelper(glyph, "Input").Set(producer);
        vtkSMPropertyHelper(glyph, "Source").Set(GetSource(glyph, source_name));
        vtkSMPropertyHelper(glyph, "ScaleMode", true).Set(scale_mode);
        vtkSMPropertyHelper(glyph, "ScaleFactor", true).Set(1.0);
        vtkSMPropertyHelper(glyph, "GlyphMode", true).Set(0);

        if (scale_mode == 0) {
          vtkSMPropertyHelper(glyph, "Scalars")
              .SetInputArrayToProcess(vtkDataObject::POINT, "diameter_");
        } else if (scale_mode == 4) {
          vtkSMPropertyHelper(glyph, "X-Scaling")
              .SetInputArrayToProcess(vtkDataObject::POINT, "diameter_");
          vtkSMPropertyHelper(glyph, "Y-Scaling")
              .SetInputArrayToProcess(vtkDataObject::POINT, "actual_length_");
          vtkSMPropertyHelper(glyph, "Z-Scaling")
              .SetInputArrayToProcess(vtkDataObject::POINT, "diameter_");
          vtkSMPropertyHelper(glyph, "Vectors")
              .SetInputArrayToProcess(vtkDataObject::POINT, "spring_axis_");
          vtkSMPropertyHelper(glyph, "MassLocation")
              .SetInputArrayToProcess(vtkDataObject::POINT, "mass_location_");
        }

        glyph->UpdateVTKObjects();
        producer->UpdateVTKObjects();
        controller_->PostInitializeProxy(glyph);
        controller_->RegisterPipelineProxy(glyph, glyph_name.c_str());

        glyph->UpdatePropertyInformation();
        glyph->UpdatePipeline();
        filter_map_[glyph_name] = glyph;

      } else if (vtk_object->IsA("vtkImageData")) {
        real_producer->SetOutput(vtkImageData::SafeDownCast(vtk_object));
      } else {
        Log::Warning("vtkCPVTKPipeline", "Object of type ",
                     vtk_object->GetClassName(),
                     " is not supported for visualization");
      }

      // Record the producer for updates
      producer_map_[object_name] = producer;
    }
  }

  // 1. Call CreatePipeline to create the producers (only once)
  // 2. For each producer in the map, update the output
  void UpdateProducers(vtkCPDataDescription* data_description) {
    // Ensure that the producers are created
    if (!pipeline_created_) {
      this->CreatePipeline(data_description);
      this->pipeline_created_ = true;
    }

    // Update the output of each recorded producer
    for (auto it = this->producer_map_.begin(); it != this->producer_map_.end();
         it++) {
      vtkObjectBase* client_side_object = it->second->GetClientSideObject();
      vtkPVTrivialProducer* real_producer =
          vtkPVTrivialProducer::SafeDownCast(client_side_object);
      real_producer->SetOutput(
          data_description->GetInputDescriptionByName(it->first.c_str())
              ->GetGrid(),
          data_description->GetTime());
    }
  }

  // 1. Call CreatePipeline to create the producers (only once)
  // 2. For each producer in the map, update the output
  void UpdateFilters(vtkCPDataDescription* data_description) {
    // Update the output of each recorded filter
    for (auto it = this->filter_map_.begin(); it != this->filter_map_.end();
         it++) {
      it->second->UpdatePipeline();
    }
  }

  void DoLiveVisualization(vtkCPDataDescription* data_description) {
    // Initialize the insitu link, if not already done
    if (!insitu_link_) {
      insitu_link_ = vtkSmartPointer<vtkLiveInsituLink>::New();
      insitu_link_->SetHostname("localhost");
      insitu_link_->SetInsituPort(22222);
      insitu_link_->Initialize(
          vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager());
    }

    // If there is not link with the ParaView client, then do not go trough the
    // process of updating sources / filters
    if (!insitu_link_->Initialize(vtkSMProxyManager::GetProxyManager()
                                      ->GetActiveSessionProxyManager())) {
      return;
    }

    // Get the time information from the data
    double time = data_description->GetTime();
    vtkIdType time_step = data_description->GetTimeStep();

    // Go into the main loop for visualization
    while (true) {
      // Initialize the link and update data
      insitu_link_->InsituUpdate(time, time_step);
      for (auto it = producer_map_.begin(); it != producer_map_.end(); it++) {
        // Update the pipeline for the current time_step
        it->second->UpdatePipeline(time);
      }
      for (auto it = this->filter_map_.begin(); it != this->filter_map_.end();
           it++) {
        it->second->UpdatePipeline(time);
      }
      insitu_link_->InsituPostProcess(time, time_step);

      // Wait for changes on ParaView side (Pause ...)
      if (insitu_link_->GetSimulationPaused()) {
        if (insitu_link_->WaitForLiveChange()) {
          break;
        }
      } else {
        break;
      }
    }
  }

  int CoProcess(vtkCPDataDescription* data_description) override {
    if (!data_description) {
      vtkWarningMacro(
          "The data description is empty. There is nothing to visualize!");
      return 0;
    }
    if (this->RequestDataDescription(data_description) == 0) {
      return 1;
    }

    // Update the producers and do live visualization
    this->UpdateProducers(data_description);
    this->UpdateFilters(data_description);
    this->DoLiveVisualization(data_description);

    return 1;
  }

  bool IsInitialized() { return initialized_; }

 private:
  bool pipeline_created_;
  vtkSMProxyManager* proxy_manager_;
  vtkSMPluginManager* plugin_manager_;
  vtkSMSessionProxyManager* session_manager_;
  std::map<std::string, vtkSmartPointer<vtkSMSourceProxy>> producer_map_;
  std::map<std::string, vtkSmartPointer<vtkSMSourceProxy>> filter_map_;
  vtkSmartPointer<vtkLiveInsituLink> insitu_link_;
  vtkSMParaViewPipelineControllerWithRendering* controller_;
  std::unordered_map<std::string, VtkSoGrid*> vtk_so_grids_;
  bool initialized_ = false;
};

#else

/// False front (to ignore Catalyst in gtests)
class vtkCPDataDescription;
class InSituPipeline {
 public:
  void Initialize() {
    Log::Fatal("vtkCPVTKPipeline::Initialize",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }
  int RequestDataDescription(vtkCPDataDescription* data_description) {
    Log::Fatal("vtkCPVTKPipeline::RequestDataDescription",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }

  void CreatePipeline(vtkCPDataDescription* data_description) {
    Log::Fatal("vtkCPVTKPipeline::CreatePipeline",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }

  void DoLiveVisualization(vtkCPDataDescription* data_description) {
    Log::Fatal("vtkCPVTKPipeline::DoLiveVisualization",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }

  int CoProcess(vtkCPDataDescription* data_description) {
    Log::Fatal("vtkCPVTKPipeline::CoProcess",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }
};

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

}  // namespace bdm

#endif  // CORE_VISUALIZATION_INSITU_PIPELINE_H_
