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

#ifndef CORE_VISUALIZATION_MAPPED_DATA_ARRAY_H_
#define CORE_VISUALIZATION_MAPPED_DATA_ARRAY_H_

#include <vtkMappedDataArray.h>
#include <vtkObjectFactory.h>
#include <vector>

#include "core/functor.h"
#include "core/sim_object/sim_object.h"

namespace bdm {

struct MappedDataArrayInterface {
  virtual void Update(const std::vector<SimObject*>* sim_objects, uint64_t start,
                      uint64_t end) = 0;
};

template <class TScalar>
class MappedDataArray : public vtkMappedDataArray<TScalar>,
                        public MappedDataArrayInterface {
 public:
  vtkAbstractTemplateTypeMacro(MappedDataArray<TScalar>,
                               vtkMappedDataArray<TScalar>)
      vtkMappedDataArrayNewInstanceMacro(
          MappedDataArray<TScalar>) static MappedDataArray* New();
  typedef typename Superclass::ValueType ValueType;

  void Initialize(const std::string& name, uint64_t num_components, Functor<TScalar*, SimObject*>* access_dm);
  void Update(const std::vector<SimObject*>* sim_objects, uint64_t start,
              uint64_t end) override;

  // Reimplemented virtuals -- see superclasses for descriptions:
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void Initialize() override;
  void GetTuples(vtkIdList* pt_ids, vtkAbstractArray* output) override;
  void GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray* output) override;
  void Squeeze() override;
  vtkArrayIterator* NewIterator() override;
  vtkIdType LookupValue(vtkVariant value) override;
  void LookupValue(vtkVariant value, vtkIdList* ids) override;
  vtkVariant GetVariantValue(vtkIdType idx) override;
  void ClearLookup() override;
  double* GetTuple(vtkIdType i) override;
  void GetTuple(vtkIdType i, double* tuple) override;
  vtkIdType LookupTypedValue(TScalar value) override;
  void LookupTypedValue(TScalar value, vtkIdList* ids) override;
  ValueType GetValue(vtkIdType idx) const override;
  TScalar& GetValueReference(vtkIdType idx) override;
  void GetTypedTuple(vtkIdType idx, TScalar* t) const override;

  // Description:
  // This container is read only -- this method does nothing but print a
  // warning.
  int Allocate(vtkIdType sz, vtkIdType ext) override;
  int Resize(vtkIdType num_tuples) override;
  void SetNumberOfTuples(vtkIdType number) override;
  void SetTuple(vtkIdType i, vtkIdType j, vtkAbstractArray* source) override;
  void SetTuple(vtkIdType i, const float* source) override;
  void SetTuple(vtkIdType i, const double* source) override;
  void InsertTuple(vtkIdType i, vtkIdType j, vtkAbstractArray* source) override;
  void InsertTuple(vtkIdType i, const float* source) override;
  void InsertTuple(vtkIdType i, const double* source) override;
  void InsertTuples(vtkIdList* dstIds, vtkIdList* srcIds,
                    vtkAbstractArray* source) override;
  void InsertTuples(vtkIdType dstStart, vtkIdType n, vtkIdType srcStart,
                    vtkAbstractArray* source) override;
  vtkIdType InsertNextTuple(vtkIdType j, vtkAbstractArray* source) override;
  vtkIdType InsertNextTuple(const float* source) override;
  vtkIdType InsertNextTuple(const double* source) override;
  void DeepCopy(vtkAbstractArray* aa) override;
  void DeepCopy(vtkDataArray* da) override;
  void InterpolateTuple(vtkIdType i, vtkIdList* ptIndices,
                        vtkAbstractArray* source, double* weights) override;
  void InterpolateTuple(vtkIdType i, vtkIdType id1, vtkAbstractArray* source1,
                        vtkIdType id2, vtkAbstractArray* source2, double t) override;
  void SetVariantValue(vtkIdType idx, vtkVariant value) override;
  void InsertVariantValue(vtkIdType idx, vtkVariant value) override;
  void RemoveTuple(vtkIdType id) override;
  void RemoveFirstTuple() override;
  void RemoveLastTuple() override;
  void SetTypedTuple(vtkIdType i, const TScalar* t) override;
  void InsertTypedTuple(vtkIdType i, const TScalar* t) override;
  vtkIdType InsertNextTypedTuple(const TScalar* t) override;
  void SetValue(vtkIdType idx, TScalar value) override;
  vtkIdType InsertNextValue(TScalar v) override;
  void InsertValue(vtkIdType idx, TScalar v) override;

 protected:
  MappedDataArray();
  ~MappedDataArray();

  /// Access sim object data member functor.
  Functor<TScalar*, SimObject*>* access_dm_= nullptr;
  const std::vector<SimObject*>* sim_objects_ = nullptr;
  uint64_t start_ = 0;
  uint64_t end_ = 0;
  TScalar* temp_array_ = nullptr;

 private:
  MappedDataArray(const MappedDataArray&) = delete;
  void operator=(const MappedDataArray&) = delete;

  vtkIdType Lookup(const TScalar& val, vtkIdType startIndex);
};

// ----------------------------------------------------------------------------
// Implementation
template <typename TScalar>
void MappedDataArray<TScalar>::Initialize(const std::string& name, uint64_t num_components, Functor<TScalar*, SimObject*>* access_dm) {
  access_dm_ = access_dm;
  this->NumberOfComponents = num_components;
  this->SetName(name.c_str());
}

template <typename TScalar>
void MappedDataArray<TScalar>::Update(const std::vector<SimObject*>* sim_objects,
                                uint64_t start, uint64_t end) {
  sim_objects_ = sim_objects;
  start_ = start;
  end_ = end;
  this->Size = this->NumberOfComponents * (end - start);
  this->MaxId = this->Size - 1;
  this->Modified();
}

//------------------------------------------------------------------------------
// Can't use vtkStandardNewMacro with a template.
template <class TScalar>
MappedDataArray<TScalar>* MappedDataArray<TScalar>::New() {
  VTK_STANDARD_NEW_BODY(MappedDataArray<TScalar>);
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::PrintSelf(ostream& os, vtkIndent indent) {
  this->MappedDataArray<TScalar>::Superclass::PrintSelf(os, indent);
  // FIXME
  // os << indent << "Array: " << this->Array << std::endl;
  // os << indent << "temp_array_: " << this->temp_array_ << std::endl;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::Initialize() {
  this->MaxId = -1;
  this->Size = 0;
  this->NumberOfComponents = 1;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::GetTuples(vtkIdList* pt_ids,
                                         vtkAbstractArray* output) {
  vtkDataArray* out_array = vtkDataArray::FastDownCast(output);
  if (!out_array) {
    vtkWarningMacro(<< "Input is not a vtkDataArray");
    return;
  }

  vtkIdType num_tuples = pt_ids->GetNumberOfIds();

  out_array->SetNumberOfComponents(this->NumberOfComponents);
  out_array->SetNumberOfTuples(num_tuples);

  for (vtkIdType i = 0; i < pt_ids->GetNumberOfIds(); ++i) {
    out_array->SetTuple(i, this->GetTuple(pt_ids->GetId(i)));
  }
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::GetTuples(vtkIdType p1, vtkIdType p2,
                                         vtkAbstractArray* output) {
  vtkDataArray* da = vtkDataArray::FastDownCast(output);
  if (!da) {
    vtkErrorMacro(<< "Input is not a vtkDataArray");
    return;
  }

  if (da->GetNumberOfComponents() != this->GetNumberOfComponents()) {
    vtkErrorMacro(<< "Incorrect number of components in input array.");
    return;
  }

  for (vtkIdType id = 0; p1 <= p2; ++p1) {
    da->SetTuple(id++, this->GetTuple(p1));
  }
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::Squeeze() {
  // noop
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkArrayIterator* MappedDataArray<TScalar>::NewIterator() {
  vtkErrorMacro(<< "Not implemented.");
  return NULL;
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::LookupValue(vtkVariant value) {
  bool valid = true;
  TScalar val = vtkVariantCast<TScalar>(value, &valid);
  if (valid) {
    return this->Lookup(val, 0);
  }
  return -1;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::LookupValue(vtkVariant value, vtkIdList* ids) {
  bool valid = true;
  TScalar val = vtkVariantCast<TScalar>(value, &valid);
  ids->Reset();
  if (valid) {
    vtkIdType index = 0;
    while ((index = this->Lookup(val, index)) >= 0) {
      ids->InsertNextId(index++);
    }
  }
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkVariant MappedDataArray<TScalar>::GetVariantValue(vtkIdType idx) {
  return vtkVariant(this->GetValueReference(idx));
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::ClearLookup() {
  // no-op, no fast lookup implemented.
}

//------------------------------------------------------------------------------
template <class TScalar>
double* MappedDataArray<TScalar>::GetTuple(vtkIdType i) {
  this->GetTuple(i, this->temp_array_);
  return this->temp_array_;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::GetTuple(vtkIdType tuple_id, double* tuple) {
  auto* data = (*access_dm_)((*sim_objects_)[start_ + tuple_id]); 
  std::cout << "GetTuple i " << tuple_id << " ";
  for(uint64_t i = 0; i < static_cast<uint64_t>(this->NumberOfComponents); ++i) {
    std::cout << data[i] << ", ";
    tuple[i] = static_cast<double>(data[i]);
  }
  std::cout << std::endl;
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::LookupTypedValue(TScalar value) {
  return this->Lookup(value, 0);
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::LookupTypedValue(TScalar value, vtkIdList* ids) {
  ids->Reset();
  vtkIdType index = 0;
  while ((index = this->Lookup(value, index)) >= 0) {
    ids->InsertNextId(index++);
  }
}

//------------------------------------------------------------------------------
template <class TScalar>
typename MappedDataArray<TScalar>::ValueType MappedDataArray<TScalar>::GetValue(
    vtkIdType idx) const {
  // Work around const-correct inconsistencies:
  typedef MappedDataArray<TScalar> ThisClass;
  return const_cast<ThisClass*>(this)->GetValueReference(idx);
}

//------------------------------------------------------------------------------
template <class TScalar>
TScalar& MappedDataArray<TScalar>::GetValueReference(vtkIdType idx) {
  const vtkIdType tuple = idx / this->NumberOfComponents;
  auto* data = (*access_dm_)((*sim_objects_)[start_ + tuple]); 
  if (this->NumberOfComponents == 1) {
    return *data;   
  }
  const vtkIdType comp = idx % this->NumberOfComponents;
  return data[comp];
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::GetTypedTuple(vtkIdType tuple_id,
                                             TScalar* tuple) const {
  auto* data = (*access_dm_)((*sim_objects_)[start_ + tuple_id]); 
  std::cout << "GetTuple i " << tuple_id << " ";
  for(uint64_t i = 0; i < static_cast<uint64_t>(this->NumberOfComponents); ++i) {
    std::cout << data[i] << ", ";
    tuple[i] = data[i];
  }
  std::cout << std::endl;
}

//------------------------------------------------------------------------------
template <class TScalar>
int MappedDataArray<TScalar>::Allocate(vtkIdType, vtkIdType) {
  vtkErrorMacro("Read only container."); return 0;
}

//------------------------------------------------------------------------------
template <class TScalar>
int MappedDataArray<TScalar>::Resize(vtkIdType) {
  vtkErrorMacro("Read only container."); return 0;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::SetNumberOfTuples(vtkIdType) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::SetTuple(vtkIdType, vtkIdType,
                                        vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::SetTuple(vtkIdType, const float*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::SetTuple(vtkIdType, const double*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertTuple(vtkIdType, vtkIdType,
                                           vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertTuple(vtkIdType, const float*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertTuple(vtkIdType, const double*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertTuples(vtkIdList*, vtkIdList*,
                                            vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertTuples(vtkIdType, vtkIdType, vtkIdType,
                                            vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::InsertNextTuple(vtkIdType,
                                                    vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::InsertNextTuple(const float*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::InsertNextTuple(const double*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::DeepCopy(vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::DeepCopy(vtkDataArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InterpolateTuple(vtkIdType, vtkIdList*,
                                                vtkAbstractArray*, double*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InterpolateTuple(vtkIdType, vtkIdType,
                                                vtkAbstractArray*, vtkIdType,
                                                vtkAbstractArray*, double) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::SetVariantValue(vtkIdType, vtkVariant) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertVariantValue(vtkIdType, vtkVariant) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::RemoveTuple(vtkIdType) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::RemoveFirstTuple() {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::RemoveLastTuple() {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::SetTypedTuple(vtkIdType, const TScalar*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertTypedTuple(vtkIdType, const TScalar*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::InsertNextTypedTuple(const TScalar*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::SetValue(vtkIdType, TScalar) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::InsertNextValue(TScalar) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <class TScalar>
void MappedDataArray<TScalar>::InsertValue(vtkIdType, TScalar) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <class TScalar>
MappedDataArray<TScalar>::MappedDataArray() {}

//------------------------------------------------------------------------------
template <class TScalar>
MappedDataArray<TScalar>::~MappedDataArray() {
  if (access_dm_) {
    delete access_dm_;
  }
}

//------------------------------------------------------------------------------
template <class TScalar>
vtkIdType MappedDataArray<TScalar>::Lookup(const TScalar& val,
                                           vtkIdType index) {
  while (index <= this->MaxId) {
    if (this->GetValueReference(index++) == val) {
      return index;
    }
  }
  return -1;
}

}  // namespace bdm

#endif  // CORE_VISUALIZATION_MAPPED_DATA_ARRAY_H_
