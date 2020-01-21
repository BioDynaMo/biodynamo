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

#ifndef CORE_PARALLEL_EXECUTION_UTIL_H_
#define CORE_PARALLEL_EXECUTION_UTIL_H_

#include <sstream>
#include <string>
#include <vector>

#include <TMessage.h>

#include "mpi.h"

using std::vector;

namespace bdm {

static const unsigned int kMaster = 0;

enum Status { kBusy, kAvail };
enum Tag { kReady, kTask, kKill };

// A struct that represents the combined containers
struct XMLParams {
  virtual ~XMLParams() {}
  template <typename T>
  void Append(vector<T> vec) {
    vector<double> v(vec.size());
    data_.push_back(v);
  }
  vector<vector<double>> GetData() const { return data_; }
  void SetData(int i, int j, double val) { data_[i][j] = val; }

 private:
  vector<vector<double>> data_;
  ClassDef(XMLParams, 1);
};

inline std::ostream& operator<<(std::ostream& os, const XMLParams& vv) {
  const auto& vec = vv.GetData();
  os << "{";
  for (auto v = vec.begin(); v != vec.end(); ++v) {
    os << "[";
    for (auto ii = v->begin(); ii != v->end(); ++ii) {
      os << *ii;
      if (ii != (--v->end()))
        os << ", ";
    }
    os << "]";
    if (v != (--vec.end()))
      os << ", ";
  }
  os << "}";
  return os;
}

struct Container {
  Container() {}
  virtual ~Container() {}
  virtual int GetNumElements() = 0;
  virtual double GetValue(int n) const = 0;
};

/// A range of values
struct Range : public Container {
  Range() {}
  Range(double min, double max, double stride)
      : min_(min), max_(max), stride_(stride){};

  // Get the nth value
  double GetValue(int n) const {
    double curr = min_ + n * stride_;
    return curr > max_ ? max_ : curr;
  }

  // Returns the number of discrete values that this range contains (including
  // the `min_` and `max_` values)
  int GetNumElements() {
    return std::round(((max_ - min_) + stride_) / stride_);
  }

  // The minimum value
  double min_ = 0;
  // THe maximum value
  double max_ = 0;
  // The stride
  double stride_ = 1;
};

struct Set : public Container, public vector<double> {
  int GetNumElements() { return this->size(); }
  double GetValue(int n) const { return this->at(n); }
};

inline void ParamGenerator(XMLParams* params, const vector<int>& slots,
                           const vector<Range>& ranges,
                           const vector<Set>& sets) {
  params->Append(ranges);
  params->Append(sets);

  size_t i = 0, j = 0;
  while (i < params->GetData()[0].size()) {
    double val = ranges[i].GetValue(slots[i]);
    params->SetData(0, i, val);
    i++;
  }

  while (j < params->GetData()[1].size()) {
    double val = sets[j].GetValue(slots[i]);
    params->SetData(1, j, val);
    i++;
    j++;
  }
}

inline vector<Container*> MergeContainers(vector<Range>* ranges,
                                          vector<Set>* sets) {
  vector<Container*> c(ranges->size() + sets->size());
  int i = 0;
  for (Range& r : *ranges) {
    c[i] = &r;
    i++;
  }
  for (Set& s : *sets) {
    c[i] = &s;
    i++;
  }
  return c;
}

/// Need this class to assign a buffer to TMessage. TMessage constructor
/// is protected. TMessage::SetBuffer doesn't do what we want. So we use this.
class MPIObject : public TMessage {
 public:
  MPIObject() = default;
  MPIObject(void* buf, Int_t len) : TMessage(buf, len) {}
};

/// Send object to worker using ROOT Serialization
template <typename T>
int MPI_Send_Obj_ROOT(T* obj, int worker, int tag) {
  MPIObject mpio;
  mpio.WriteObject(obj);
  int size = mpio.Length();
  // First send the size of the buffer
  MPI_Send(&size, 1, MPI_INT, worker, tag, MPI_COMM_WORLD);
  // Then send the buffer
  return MPI_Send(mpio.Buffer(), size, MPI_BYTE, worker, tag, MPI_COMM_WORLD);
}

/// Receive object from master using ROOT Serialization
template <typename T>
T* MPI_Recv_Obj_ROOT(int size, int tag, MPI_Status& status) {
  char* buf = (char*)malloc(size);
  // Then receive the buffer
  MPI_Recv(buf, size, MPI_BYTE, kMaster, tag, MPI_COMM_WORLD, &status);
  MPIObject* mpio = new MPIObject(buf, size);
  T* obj = (T*)(mpio->ReadObject(mpio->GetClass()));
  free(buf);
  return obj;
}

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_UTIL_H_
