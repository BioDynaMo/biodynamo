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

#ifndef CORE_EXPORTER_H_
#define CORE_EXPORTER_H_

#include <memory>
#include <string>

namespace bdm {

class Exporter {
 public:
  virtual ~Exporter();

  /// Export the simulation state of one iteration
  /// \param filename
  /// \param iteration - current iteration number (=time step)
  virtual void ExportIteration(std::string filename, uint64_t iteration) = 0;

  /// Export the simulation summary
  /// \param filename
  /// \param num_iterations - total number of iterations
  virtual void ExportSummary(std::string filename, uint64_t num_iterations) = 0;
};

class BasicExporter : public Exporter {
 public:
  void ExportIteration(std::string filename, uint64_t iteration) override;

  void ExportSummary(std::string filename, uint64_t num_iterations) override;
};

class MatlabExporter : public Exporter {
 public:
  void ExportIteration(std::string filename, uint64_t iteration) override;

  void ExportSummary(std::string filename, uint64_t num_iterations) override;
};

class NeuroMLExporter : public Exporter {
 public:
  void ExportIteration(std::string filename, uint64_t iteration) override;

  void ExportSummary(std::string filename, uint64_t num_iterations) override;
};

class ParaviewExporter : public Exporter {
 public:
  void ExportIteration(std::string filename, uint64_t iteration) override;

  /// This function creates a .pvd file that lists the individual files across
  /// the different times.
  /// This .pvd can be read by Paraview for visualization.
  void ExportSummary(std::string filename, uint64_t num_iterations) override;
};

enum ExporterType { kBasic, kMatlab, kNeuroML, kParaview };

class ExporterFactory {
 public:
  static std::unique_ptr<Exporter> GenerateExporter(ExporterType type);
};

}  // namespace bdm

#endif  // CORE_EXPORTER_H_
