// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_MULTI_SIMULATION_MPI_HELPER_H_
#define CORE_MULTI_SIMULATION_MPI_HELPER_H_

#include <sstream>
#include <string>
#include <vector>

#include <TMessage.h>

// Hide MPI headers from Cling
#if (!defined(__CLING__) && !defined(__ROOTCLING__))
#include "mpi.h"
#endif  // __ROOTCLING__

#include "core/util/log.h"
#include "core/util/root.h"

namespace bdm {
namespace experimental {

#ifdef USE_MPI

/// Need this class to assign a buffer to TMessage. TMessage constructor
/// is protected. TMessage::SetBuffer doesn't do what we want. So we use this.
class MPIObject : public TMessage {
 public:
  MPIObject() = default;
  ~MPIObject() = default;
  MPIObject(void* buf, Int_t len) : TMessage(buf, len) {}

 private:
  BDM_CLASS_DEF(MPIObject, 1);
};

// Hide MPI functions from Cling
#if (!defined(__CLING__) && !defined(__ROOTCLING__))
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

#endif  // __ROOTCLING__

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_MULTI_SIMULATION_MPI_HELPER_H_
