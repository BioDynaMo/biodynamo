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

#ifndef CORE_ENVIRONMENT_ENVIRONMENT_H_
#define CORE_ENVIRONMENT_ENVIRONMENT_H_

#include <array>

#include "core/functor.h"
#include "core/sim_object/sim_object.h"
#include "core/sim_object/so_handle.h"

namespace bdm {

class Environment {
 public:
  virtual ~Environment();

  virtual void Update() = 0;

  virtual void ForEachNeighbor(Functor<void, const SimObject*, double>& lambda,
                               const SimObject& query) = 0;

  virtual void Clear() = 0;

  virtual const std::array<int32_t, 6>& GetDimensions() const = 0;

  virtual const std::array<int32_t, 2>& GetDimensionThresholds() const = 0;

  /// Return the size of the largest sim object
  virtual double GetLargestObjectSize() const = 0;

  virtual void IterateZOrder(Functor<void, const SoHandle&>& callback) = 0;

  /// This class ensures thread-safety for the case
  /// that a simulation object modifies its neighbors.
  class NeighborMutexBuilder {
   public:
    /// The NeighborMutex class is a synchronization primitive that can be
    /// used to protect sim_objects data from being simultaneously accessed by
    /// multiple threads.
    class NeighborMutex {
     public:
      virtual ~NeighborMutex() {}
      virtual void lock() {}    // NOLINT
      virtual void unlock() {}  // NOLINT
    };

    virtual ~NeighborMutexBuilder() {}
    virtual NeighborMutex* GetMutex(uint64_t box_idx) = 0;
  };

  /// Returns the `NeighborMutexBuilder`. The client uses it to create a
  /// `NeighborMutex`.
  virtual NeighborMutexBuilder* GetNeighborMutexBuilder() = 0;

  bool HasGrown() const;

 protected:
  bool has_grown_ = false;

  struct SimDimensionAndLargestSimObjectFunctor;

  /// Calculates what the grid dimensions need to be in order to contain all the
  /// simulation objects
  void CalcSimDimensionsAndLargestSimObject(
      std::array<double, 6>* ret_grid_dimensions, double* largest_so);

};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_ENVIRONMENT_H_
