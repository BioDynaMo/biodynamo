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

#ifndef CORE_MULTI_SIMULATION_UTIL_H_
#define CORE_MULTI_SIMULATION_UTIL_H_

#ifdef USE_MPI

#include <sstream>
#include <string>
#include <vector>

#include <TMessage.h>

#include "mpi.h"

#include "core/util/log.h"
#include "core/util/root.h"

using std::vector;

namespace bdm {

static const unsigned int kMaster = 0;

enum Status { kBusy, kAvail };
enum Tag { kReady, kTask, kKill };

struct Container {
  Container() {}
  explicit Container(const std::string& name) : param_name_(name) {}
  virtual ~Container() {}

  virtual Container* GetCopy() const = 0;

  virtual int GetNumElements() const = 0;
  virtual double GetValue(int n) const = 0;
  virtual void Validate() const {};

  // Return the substring before the last "::", which should be
  // bdm::<ParamGroup>
  std::string GetGroupName() {
    size_t found = param_name_.find_last_of("::");
    return param_name_.substr(0, found - 1);
  }

  // Return the substring after the last "::", which should be <param_name>
  std::string GetParamName() {
    size_t found = param_name_.find_last_of("::");
    return param_name_.substr(found + 1);
  }

  // Must be in format bdm::<ParamGroup>::<param_name>
  std::string param_name_;
  BDM_CLASS_DEF(Container, 1);
};

/// A range of values
struct Range : public Container {
  Range() {}
  Range(const std::string& n, double min, double max, double stride)
      : Container(n), lower_bound_(min), upper_bound_(max), stride_(stride) {
    Validate();
  };

  void Validate() const override {
    if (lower_bound_ > upper_bound_) {
      Log::Fatal("Range", "Tried to initialize parameter '", param_name_,
                 "' with a lower_bound_ value higher than upper_bound_: ",
                 lower_bound_, " > ", upper_bound_);
    }
  }

  Container* GetCopy() const override { return new Range(*this); }

  // Get the nth value
  double GetValue(int n) const override {
    double curr = lower_bound_ + n * stride_;
    return curr > upper_bound_ ? upper_bound_ : curr;
  }

  // Returns the number of discrete values that this range contains (including
  // the `lower_bound_` and `upper_bound_` values)
  int GetNumElements() const override {
    return std::round(((upper_bound_ - lower_bound_) + stride_) / stride_);
  }

  // The minimum value
  double lower_bound_ = 0;
  // THe maximum value
  double upper_bound_ = 0;
  // The stride
  double stride_ = 1;
  BDM_CLASS_DEF_OVERRIDE(Range, 1);
};

/// A range of values
struct LogRange : public Container {
  LogRange() {}
  LogRange(const std::string& n, double base, double min, double max,
           double stride)
      : Container(n),
        base_(base),
        lower_bound_(min),
        upper_bound_(max),
        stride_(stride) {
    Validate();
  };

  void Validate() const override {
    if (lower_bound_ > upper_bound_) {
      Log::Fatal("LogRange", "Tried to initialize parameter '", param_name_,
                 "' with a lower_bound_ value higher than upper_bound_: ",
                 lower_bound_, " > ", upper_bound_);
    }
  }

  Container* GetCopy() const override { return new LogRange(*this); }

  // Get the nth value
  double GetValue(int n) const override {
    double exp = lower_bound_ + n * stride_;
    return exp > upper_bound_ ? std::pow(base_, upper_bound_)
                              : std::pow(base_, exp);
  }

  // Returns the number of discrete values that this range contains (including
  // the `lower_bound_` and `upper_bound_` values)
  int GetNumElements() const override {
    return std::round(((upper_bound_ - lower_bound_) + stride_) / stride_);
  }

  // The base value
  double base_ = 10;
  // The minimum value
  double lower_bound_ = 0;
  // THe maximum value
  double upper_bound_ = 0;
  // The stride
  double stride_ = 1;
  BDM_CLASS_DEF_OVERRIDE(LogRange, 1);
};

struct Set : public Container {
  Set() {}
  Set(const std::string& n, const std::vector<double> v)
      : Container(n), values_(v) {}

  Container* GetCopy() const override { return new Set(*this); }

  size_t size() const { return values_.size(); }
  double at(size_t n) const { return values_.at(n); }

  int GetNumElements() const override { return this->size(); }
  double GetValue(int n) const override { return this->at(n); }

  std::vector<double> values_;
  BDM_CLASS_DEF_OVERRIDE(Set, 1);
};

/// Need this class to assign a buffer to TMessage. TMessage constructor
/// is protected. TMessage::SetBuffer doesn't do what we want. So we use this.
class MPIObject : public TMessage {
 public:
  MPIObject() = default;
  MPIObject(void* buf, Int_t len) : TMessage(buf, len) {}

 private:
  BDM_CLASS_DEF(MPIObject, 1);
};

/// Send object to worker using ROOT Serialization
template <typename T>
int MPI_Send_Obj_ROOT(T* obj, int dest, int tag,
                      MPI_Status* status = MPI_STATUS_IGNORE) {
  MPIObject mpio;
  mpio.WriteObject(obj);
  int size = mpio.Length();
  // First send the size of the buffer
  MPI_Send(&size, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
  // Then send the buffer
  return MPI_Send(mpio.Buffer(), size, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
}

/// Receive object from master using ROOT Serialization
template <typename T>
T* MPI_Recv_Obj_ROOT(int size, int source, int tag,
                     MPI_Status* status = MPI_STATUS_IGNORE) {
  char* buf = (char*)malloc(size);
  // Then receive the buffer
  MPI_Recv(buf, size, MPI_BYTE, source, tag, MPI_COMM_WORLD, status);
  MPIObject* mpio = new MPIObject(buf, size);
  T* obj = (T*)(mpio->ReadObject(mpio->GetClass()));
  free(buf);
  return obj;
}

}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_MULTI_SIMULATION_UTIL_H_
