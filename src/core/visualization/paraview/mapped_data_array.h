//  -----------------------------------------------------------------------------
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
#include "core/param/param.h"
#include "core/sim_object/sim_object.h"
#include "core/sim_object/so_pointer.h"
#include "core/sim_object/sim_object.h"
#include "core/util/type.h"

namespace bdm {

// -----------------------------------------------------------------------------
// FIXME document
template <typename TReturn, typename TClass, typename TDataMember>
struct GetDataMemberForVis {
  uint64_t dm_offset_;
  mutable typename std::remove_pointer<TReturn>::type temp_value;

  enum DataType { kDefault, kArray, kSoUid, kSoPointer };

  template <typename T> 
  static constexpr DataType GetDataType() {
    if (IsArray<T>::value) {
      return DataType::kArray;
    } else if (std::is_same<T, SoUid>::value) {
      return DataType::kSoUid;
    } else if (is_so_ptr<T>::value) {
      return DataType::kSoPointer;
    }
    return DataType::kDefault;
  };

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<GetDataType<TTDataMember>() == DataType::kDefault, TReturn>::type
  operator()(SimObject* so) const {
    auto* casted_so = static_cast<TClass*>(so);
    return reinterpret_cast<TDataMember*>(
                     reinterpret_cast<char*>(casted_so) + dm_offset_);
  }

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<GetDataType<TTDataMember>() == DataType::kArray, TReturn>::type
  operator()(SimObject* so) const {
    auto* casted_so = static_cast<TClass*>(so);
    auto* data = reinterpret_cast<TDataMember*>(
                     reinterpret_cast<char*>(casted_so) + dm_offset_)
                     ->data();
    return const_cast<TReturn>(data);
  }

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<GetDataType<TTDataMember>() == DataType::kSoUid, TReturn>::type
  operator()(SimObject* so) const {
    auto* casted_so = static_cast<TClass*>(so);
    auto* data = reinterpret_cast<TDataMember*>(
                     reinterpret_cast<char*>(casted_so) + dm_offset_);
    uint64_t uid = *data;
    temp_value = uid;
    return &temp_value;
  }

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<GetDataType<TTDataMember>() == DataType::kSoPointer, TReturn>::type
  operator()(SimObject* so) const {
    auto* casted_so = static_cast<TClass*>(so);
    auto* data = reinterpret_cast<TDataMember*>(
                     reinterpret_cast<char*>(casted_so) + dm_offset_);
    uint64_t uid = data->GetUid();
    temp_value = uid;
    return &temp_value;
  }
};

// -----------------------------------------------------------------------------
struct MappedDataArrayInterface {
  virtual void Update(const std::vector<SimObject*>* sim_objects, uint64_t start,
                      uint64_t end) = 0;
};

// -----------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
class MappedDataArray : public vtkMappedDataArray<TScalar>,
                        public MappedDataArrayInterface {
 public:
  vtkAbstractTemplateTypeMacro(MappedDataArray,
                               vtkMappedDataArray<TScalar>)
      vtkMappedDataArrayNewInstanceMacro(
          MappedDataArray) static MappedDataArray* New();
  typedef typename Superclass::ValueType ValueType;

  void Initialize(Param::MappedDataArrayMode mode, const std::string& name, uint64_t num_components, uint64_t dm_offset);
  void Update(const std::vector<SimObject*>* sim_objects, uint64_t start,
              uint64_t end) final;

  // Reimplemented virtuals -- see superclasses for descriptions:
  void PrintSelf(ostream& os, vtkIndent indent) final;
  void Initialize() final;
  void GetTuples(vtkIdList* pt_ids, vtkAbstractArray* output) final;
  void GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray* output) final;
  void Squeeze() final;
  vtkArrayIterator* NewIterator() final;
  vtkIdType LookupValue(vtkVariant value) final;
  void LookupValue(vtkVariant value, vtkIdList* ids) final;
  vtkVariant GetVariantValue(vtkIdType idx) final;
  void ClearLookup() final;
  double* GetTuple(vtkIdType i) final;
  void GetTuple(vtkIdType i, double* tuple) final;
  vtkIdType LookupTypedValue(TScalar value) final;
  void LookupTypedValue(TScalar value, vtkIdList* ids) final;
  ValueType GetValue(vtkIdType idx) const final;
  TScalar& GetValueReference(vtkIdType idx) final;
  void GetTypedTuple(vtkIdType idx, TScalar* t) const final;

  // Description:
  // This container is read only -- this method does nothing but print a
  // warning.
  int Allocate(vtkIdType sz, vtkIdType ext) final;
  int Resize(vtkIdType num_tuples) final;
  void SetNumberOfTuples(vtkIdType number) final;
  void SetTuple(vtkIdType i, vtkIdType j, vtkAbstractArray* source) final;
  void SetTuple(vtkIdType i, const float* source) final;
  void SetTuple(vtkIdType i, const double* source) final;
  void InsertTuple(vtkIdType i, vtkIdType j, vtkAbstractArray* source) final;
  void InsertTuple(vtkIdType i, const float* source) final;
  void InsertTuple(vtkIdType i, const double* source) final;
  void InsertTuples(vtkIdList* dstIds, vtkIdList* srcIds,
                    vtkAbstractArray* source) final;
  void InsertTuples(vtkIdType dstStart, vtkIdType n, vtkIdType srcStart,
                    vtkAbstractArray* source) final;
  vtkIdType InsertNextTuple(vtkIdType j, vtkAbstractArray* source) final;
  vtkIdType InsertNextTuple(const float* source) final;
  vtkIdType InsertNextTuple(const double* source) final;
  void DeepCopy(vtkAbstractArray* aa) final;
  void DeepCopy(vtkDataArray* da) final;
  void InterpolateTuple(vtkIdType i, vtkIdList* ptIndices,
                        vtkAbstractArray* source, double* weights) final;
  void InterpolateTuple(vtkIdType i, vtkIdType id1, vtkAbstractArray* source1,
                        vtkIdType id2, vtkAbstractArray* source2, double t) final;
  void SetVariantValue(vtkIdType idx, vtkVariant value) final;
  void InsertVariantValue(vtkIdType idx, vtkVariant value) final;
  void RemoveTuple(vtkIdType id) final;
  void RemoveFirstTuple() final;
  void RemoveLastTuple() final;
  void SetTypedTuple(vtkIdType i, const TScalar* t) final;
  void InsertTypedTuple(vtkIdType i, const TScalar* t) final;
  vtkIdType InsertNextTypedTuple(const TScalar* t) final;
  void SetValue(vtkIdType idx, TScalar value) final;
  vtkIdType InsertNextValue(TScalar v) final;
  void InsertValue(vtkIdType idx, TScalar v) final;

 protected:
  MappedDataArray();
  ~MappedDataArray();

  /// Access sim object data member functor.
  GetDataMemberForVis<TScalar*, TClass, TDataMember> get_dm_;
  const std::vector<SimObject*>* sim_objects_ = nullptr;
  uint64_t start_ = 0;
  uint64_t end_ = 0;
  double* temp_array_ = nullptr;
  
  Param::MappedDataArrayMode mode_;
  mutable uint16_t match_value = true;
  mutable std::vector<uint64_t> is_matching_;
  mutable std::vector<TScalar> data_;

 private:
  MappedDataArray(const MappedDataArray&) = delete;
  void operator=(const MappedDataArray&) = delete;

  vtkIdType Lookup(const TScalar& val, vtkIdType startIndex);
};

// ----------------------------------------------------------------------------
// Implementation
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::Initialize(Param::MappedDataArrayMode mode, const std::string& name, uint64_t num_components, uint64_t dm_offset) {
  get_dm_.dm_offset_ = dm_offset;
  mode_ = mode;
  this->NumberOfComponents = num_components;
  this->SetName(name.c_str());
}

template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::Update(const std::vector<SimObject*>* sim_objects,
                                uint64_t start, uint64_t end) {
  sim_objects_ = sim_objects;
  start_ = start;
  end_ = end;
  this->Size = this->NumberOfComponents * (end - start);
  this->MaxId = this->Size - 1;

  if (this->Size <= 0) {
    this->Size = 0;
    this->MaxId = -1;
    this->Modified();
    return;
  }

  this->Modified();

  if (mode_ != Param::MappedDataArrayMode::kZeroCopy) {
    if (data_.capacity() < this->Size) {
      data_.reserve(this->Size * 1.5);
    }
    if (mode_ == Param::MappedDataArrayMode::kCopy) {
      uint64_t counter = 0;
      for(uint64_t i = start; i < end; ++i) {
        auto* data = get_dm_((*sim_objects_)[i]);
        for (uint64_t c = 0; c < this->NumberOfComponents; ++c) {
          data_[counter++] = data[c];
        }
      } 
    } else {
      // mode_ == Param::MappedDataArrayMode::kCache
      auto cap = is_matching_.capacity();
      if (cap < this->Size) {
        is_matching_.reserve(this->Size * 1.5);
        for (uint64_t i = cap; i < is_matching_.capacity(); ++i) {
          is_matching_[i] = match_value;
        }
      }
      match_value++;
    }
  }  
}

//------------------------------------------------------------------------------
// Can't use vtkStandardNewMacro with a template.
template <typename TScalar, typename TClass, typename TDataMember>
MappedDataArray<TScalar, TClass, TDataMember>* MappedDataArray<TScalar, TClass, TDataMember>::New() {
  VTK_STANDARD_NEW_BODY(MappedDataArray);
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::PrintSelf(ostream& os, vtkIndent indent) {
  this->MappedDataArray<TScalar, TClass, TDataMember>::Superclass::PrintSelf(os, indent);
  // FIXME
  // os << indent << "Array: " << this->Array << std::endl;
  // os << indent << "temp_array_: " << this->temp_array_ << std::endl;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::Initialize() {
  this->MaxId = -1;
  this->Size = 0;
  this->NumberOfComponents = 1;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::GetTuples(vtkIdList* pt_ids,
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
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::GetTuples(vtkIdType p1, vtkIdType p2,
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
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::Squeeze() {
  // noop
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkArrayIterator* MappedDataArray<TScalar, TClass, TDataMember>::NewIterator() {
  vtkErrorMacro(<< "Not implemented.");
  return NULL;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::LookupValue(vtkVariant value) {
  bool valid = true;
  TScalar val = vtkVariantCast<TScalar>(value, &valid);
  if (valid) {
    return this->Lookup(val, 0);
  }
  return -1;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::LookupValue(vtkVariant value, vtkIdList* ids) {
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
template <typename TScalar, typename TClass, typename TDataMember>
vtkVariant MappedDataArray<TScalar, TClass, TDataMember>::GetVariantValue(vtkIdType idx) {
  return vtkVariant(this->GetValueReference(idx));
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::ClearLookup() {
  // no-op, no fast lookup implemented.
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
double* MappedDataArray<TScalar, TClass, TDataMember>::GetTuple(vtkIdType i) {
  this->GetTuple(i, this->temp_array_);
  return this->temp_array_;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::GetTuple(vtkIdType tuple_id, double* tuple) {
  uint64_t idx = tuple_id * this->NumberOfComponents;
  TScalar* data;
  switch(mode_) {
    case Param::MappedDataArrayMode::kCache:
      { 
        bool all_matches = true;
        for(uint64_t i = idx; i < idx + static_cast<uint64_t>(this->NumberOfComponents); ++i) {
          all_matches |= (is_matching_[i] == match_value);
        }
        if (all_matches) {
          uint64_t cnt = 0;
          for(uint64_t i = idx; i < idx + static_cast<uint64_t>(this->NumberOfComponents); ++i) {
            tuple[cnt++] = static_cast<double>(data_[i]);
          }
          return;
        }
      }
    case Param::MappedDataArrayMode::kZeroCopy:
      {
        auto* data = get_dm_((*sim_objects_)[start_ + tuple_id]); 
        for(uint64_t i = 0; i < static_cast<uint64_t>(this->NumberOfComponents); ++i) {
          tuple[i] = static_cast<double>(data[i]);
          if (mode_ == Param::MappedDataArrayMode::kCache) {
            data_[idx + i] = data[i];
            is_matching_[idx + i] = match_value;
          }
        }
        return;
      }
      break;
    case Param::MappedDataArrayMode::kCopy:
      uint64_t cnt = 0;
      for(uint64_t i = idx; i < idx + static_cast<uint64_t>(this->NumberOfComponents); ++i) {
        tuple[cnt++] = static_cast<double>(data_[i]);
      }
  }
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::LookupTypedValue(TScalar value) {
  return this->Lookup(value, 0);
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::LookupTypedValue(TScalar value, vtkIdList* ids) {
  ids->Reset();
  vtkIdType index = 0;
  while ((index = this->Lookup(value, index)) >= 0) {
    ids->InsertNextId(index++);
  }
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
typename MappedDataArray<TScalar, TClass, TDataMember>::ValueType MappedDataArray<TScalar, TClass, TDataMember>::GetValue(
    vtkIdType idx) const {
  //FIXME code duplication with GetValueReference 
  // make non virtual private helper function and call it from here and GetValueReference
  switch(mode_) {
    case Param::MappedDataArrayMode::kCache:
      if (is_matching_[idx] == match_value) {
        return data_[idx];
      }
    case Param::MappedDataArrayMode::kZeroCopy:
      {
      if (this->NumberOfComponents == 1) {
        auto* data = get_dm_((*sim_objects_)[start_ + idx]); 
        if (mode_ == Param::MappedDataArrayMode::kCache) {
          data_[idx] = *data;
          is_matching_[idx] = match_value;
        }
        return *data;
      }
      const vtkIdType tuple = idx / this->NumberOfComponents;
      const vtkIdType comp = idx % this->NumberOfComponents;
      auto* data = get_dm_((*sim_objects_)[start_ + tuple]); 
      if (mode_ == Param::MappedDataArrayMode::kCache) {
        data_[idx] = data[comp];
        is_matching_[idx] = match_value;
      }
      return data[comp];
      }
      break;
    case Param::MappedDataArrayMode::kCopy:
      return data_[idx];
  }
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
TScalar& MappedDataArray<TScalar, TClass, TDataMember>::GetValueReference(vtkIdType idx) {
  switch(mode_) {
    case Param::MappedDataArrayMode::kCache:
      if (is_matching_[idx] == match_value) {
        return data_[idx];
      }
    case Param::MappedDataArrayMode::kZeroCopy:
      {
      if (this->NumberOfComponents == 1) {
        auto* data = get_dm_((*sim_objects_)[start_ + idx]); 
        if (mode_ == Param::MappedDataArrayMode::kCache) {
          data_[idx] = *data;
          is_matching_[idx] = match_value;
        }
        return *data;
      }
      const vtkIdType tuple = idx / this->NumberOfComponents;
      const vtkIdType comp = idx % this->NumberOfComponents;
      auto* data = get_dm_((*sim_objects_)[start_ + tuple]); 
      if (mode_ == Param::MappedDataArrayMode::kCache) {
        data_[idx] = data[comp];
        is_matching_[idx] = match_value;
      }
      return data[comp];
      }
      break;
    case Param::MappedDataArrayMode::kCopy:
      return data_[idx];
  }
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::GetTypedTuple(vtkIdType tuple_id,
                                             TScalar* tuple) const {
  auto* data = get_dm_((*sim_objects_)[start_ + tuple_id]); 
  std::cout << "GetTuple i " << tuple_id << " ";
  for(uint64_t i = 0; i < static_cast<uint64_t>(this->NumberOfComponents); ++i) {
    std::cout << data[i] << ", ";
    tuple[i] = data[i];
  }
  std::cout << std::endl;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
int MappedDataArray<TScalar, TClass, TDataMember>::Allocate(vtkIdType, vtkIdType) {
  vtkErrorMacro("Read only container."); return 0;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
int MappedDataArray<TScalar, TClass, TDataMember>::Resize(vtkIdType) {
  vtkErrorMacro("Read only container."); return 0;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::SetNumberOfTuples(vtkIdType) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::SetTuple(vtkIdType, vtkIdType,
                                        vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::SetTuple(vtkIdType, const float*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::SetTuple(vtkIdType, const double*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertTuple(vtkIdType, vtkIdType,
                                           vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertTuple(vtkIdType, const float*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertTuple(vtkIdType, const double*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertTuples(vtkIdList*, vtkIdList*,
                                            vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertTuples(vtkIdType, vtkIdType, vtkIdType,
                                            vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::InsertNextTuple(vtkIdType,
                                                    vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::InsertNextTuple(const float*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::InsertNextTuple(const double*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::DeepCopy(vtkAbstractArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::DeepCopy(vtkDataArray*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InterpolateTuple(vtkIdType, vtkIdList*,
                                                vtkAbstractArray*, double*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InterpolateTuple(vtkIdType, vtkIdType,
                                                vtkAbstractArray*, vtkIdType,
                                                vtkAbstractArray*, double) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::SetVariantValue(vtkIdType, vtkVariant) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertVariantValue(vtkIdType, vtkVariant) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::RemoveTuple(vtkIdType) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::RemoveFirstTuple() {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::RemoveLastTuple() {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::SetTypedTuple(vtkIdType, const TScalar*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertTypedTuple(vtkIdType, const TScalar*) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::InsertNextTypedTuple(const TScalar*) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::SetValue(vtkIdType, TScalar) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::InsertNextValue(TScalar) {
  vtkErrorMacro("Read only container."); return -1;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
void MappedDataArray<TScalar, TClass, TDataMember>::InsertValue(vtkIdType, TScalar) {
  vtkErrorMacro("Read only container."); return;
}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
MappedDataArray<TScalar, TClass, TDataMember>::MappedDataArray() {}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
MappedDataArray<TScalar, TClass, TDataMember>::~MappedDataArray() {}

//------------------------------------------------------------------------------
template <typename TScalar, typename TClass, typename TDataMember>
vtkIdType MappedDataArray<TScalar, TClass, TDataMember>::Lookup(const TScalar& val,
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
