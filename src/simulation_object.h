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

#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "log.h"

#include "backend.h"
#include "biology_module_util.h"
#include "event/event.h"
#include "root_util.h"
#include "sim_object/so_uid.h"
#include "simulation_object_util.h"
#include "type_util.h"

namespace bdm {

using std::enable_if;
using std::is_same;

template <typename TCompileTimeParam, typename TDerived>
class ScalarSimulationObject;

/// Contains implementation for SimulationObject that are specific to SOA
/// backend. The peculiarity of SOA objects is that it is simulation object
/// and container at the same time.
template <typename TCompileTimeParam, typename TDerived>
class SoaSimulationObject {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  template <typename, typename>
  friend class SoaSimulationObject;

  template <typename TTBackend>
  using MostDerived = typename TDerived::template type<
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;

  template <typename TBackend>
  using Self =
      SoaSimulationObject<typename TCompileTimeParam::template Self<TBackend>,
                          TDerived>;

  SoaSimulationObject() : size_(1) {}

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimulationObject(const Self<SoaRef> &other);

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimulationObject(const Self<Soa> &other);

  template <typename T>
  SoaSimulationObject(T *other, size_t idx) : kIdx(idx), size_(other->size_) {}

  virtual ~SoaSimulationObject() {}

  SoaSimulationObject &operator=(SoaSimulationObject &&other) {
    size_ = other.size_;
    return *this;
  }

  SoaSimulationObject& operator=(const Self<Soa>& other) {
    size_ = other.size_;
    return *this;
  }

  SoaSimulationObject& operator=(const Self<SoaRef> &) {
    // Do not copy size! This is used to assign: soa[idx] = soaref;
    return *this;
  }

  Self<Backend> &operator=(
      const ScalarSimulationObject<
          typename TCompileTimeParam::template Self<Scalar>, TDerived> &) {
    // Do not copy size! This is used to assign: soa[idx] = scalar;
    return *this;
  }

  uint32_t GetElementIdx() const { return kIdx; }

  void SetElementIdx(uint32_t element_idx) {}

  /// Returns the vector's size. Uncommited changes are not taken into account
  size_t size() const {  // NOLINT
    return size_;
  }

  template <typename TTBackend>
  void push_back(const MostDerived<TTBackend> &element) {  // NOLINT
    PushBackImpl(element);
  }

  void pop_back() {  // NOLINT
    PopBack();
  }

  /// Equivalent to std::vector<> clear - it removes all elements from all
  /// data members
  void clear() {  // NOLINT
    size_ = 0;
  }

  /// Equivalent to std::vector<> reserve - it increases the capacity
  /// of all data member containers
  void reserve(size_t new_capacity) {}  // NOLINT

  /// Equivalent to std::vector<> resize
  void resize(size_t new_size) {  // NOLINT
    size_ = new_size;
  }

  template <typename Function>
  void ForEachDataMember(Function l) {}

  template <typename Function>
  void ForEachDataMemberIn(const std::set<std::string> &dm_selector,
                           Function f) {
    // validate data_members
    // all data members should have been removed from the set. Remaining
    // entries do not exist
    if (dm_selector.size() != 0) {
      std::stringstream sstr;
      for (auto &element : dm_selector) {
        sstr << element << ", ";
      }
      Fatal("ForEachDataMemberIn",
            "Please check your config file. The following data members do not "
            "exist: %s",
            sstr.str().c_str());
    }
  }

 protected:
  const size_t kIdx = 0;

  /// Append a scalar element
  virtual void PushBackImpl(const MostDerived<Scalar> &other) { size_++; }

  /// Append a soa ref element
  virtual void PushBackImpl(const MostDerived<SoaRef> &other) { size_++; }

  /// Remove last element
  virtual void PopBack() { size_--; }

 private:
  /// size_ is of type size_t& if Backend == SoaRef; otherwise size_t
  typename type_ternary_operator<is_same<Backend, SoaRef>::value, size_t &,
                                 size_t>::type size_;

  // use modified class def, due to possible SoaRef backend
  BDM_ROOT_CLASS_DEF(SoaSimulationObject, 1);
};

/// Contains implementations for SimulationObject that are specific to scalar
/// backend
template <typename TCompileTimeParam, typename TDerived>
class ScalarSimulationObject {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  template <typename TTBackend>
  using MostDerived = typename TDerived::template type<
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;

  ScalarSimulationObject() : element_idx_(0) {}
  ScalarSimulationObject(const ScalarSimulationObject &other)
      : element_idx_(other.element_idx_) {}

  virtual ~ScalarSimulationObject() {}

  ScalarSimulationObject &operator=(ScalarSimulationObject &&other) {
    element_idx_ = other.element_idx_;
    return *this;
  }

  ScalarSimulationObject &operator=(const ScalarSimulationObject &) {
    return *this;
  }

  std::size_t size() const { return 1; }  // NOLINT

  /// NB: Cannot be used in the Constructur, because the ResourceManager`
  /// didn't initialize `element_idx_` yet.
  uint32_t GetElementIdx() const { return element_idx_; }

  // assign the array index of this object in the ResourceManager
  void SetElementIdx(uint32_t element_idx) { element_idx_ = element_idx; }

 protected:
  static const std::size_t kIdx = 0;
  // array index of this object in the ResourceManager
  uint32_t element_idx_ = 0;

  /// Append a scalar element
  virtual void PushBackImpl(const MostDerived<Scalar> &other) {}

  /// Append a SoaRef element
  virtual void PushBackImpl(const MostDerived<SoaRef> &other) {}

  /// Remove last element
  virtual void PopBack() {}

  BDM_ROOT_CLASS_DEF(ScalarSimulationObject, 2);
};

/// Helper type trait to map backends to simulation object implementations
template <typename TCompileTimeParam, typename TDerived>
struct SimulationObjectImpl {
  using Backend = typename TCompileTimeParam::Backend;
  using type = typename type_ternary_operator<
      is_same<Backend, Scalar>::value,
      ScalarSimulationObject<TCompileTimeParam, TDerived>,
      SoaSimulationObject<TCompileTimeParam, TDerived>>::type;
};

/// Contains code required by all simulation objects
template <typename TCompileTimeParam, typename TDerived>
class SimulationObjectExt
    : public SimulationObjectImpl<TCompileTimeParam, TDerived>::type {
  // used to fullfill BDM_SIM_OBJECT_HEADER requirement
  template <typename T, typename U>
  using SimObjectBaseExt = typename SimulationObjectImpl<T, U>::type;

  BDM_SIM_OBJECT_HEADER(SimulationObject, SimObjectBase, 1, uid_, box_idx_,
                        biology_modules_, run_bm_loop_idx_, numa_node_);

 public:
  SimulationObjectExt() : Base() {
    uid_[kIdx] = SoUidGenerator::Get()->NewSoUid();
  }

  template <typename TEvent, typename TOther>
  SimulationObjectExt(const TEvent &event, TOther *other,
                      uint64_t new_oid = 0) {
    uid_[kIdx] = SoUidGenerator::Get()->NewSoUid();
    box_idx_[kIdx] = other->GetBoxIdx();
    // biology modules
    auto &other_bms = other->biology_modules_[other->kIdx];
    // copy biology_modules_ to me
    CopyBiologyModules(event, &other_bms, &biology_modules_[kIdx]);
  }

  virtual ~SimulationObjectExt() {}

  /// This function determines if the type of this simulation object is the same
  /// as `TSo` without taking the backend into account.
  /// @tparam TSo Simulation object type with any backend
  template <typename TSo>
  static constexpr bool IsSoType() {
    using TSoScalar = typename TSo::template Self<Scalar>;
    return std::is_same<MostDerived<Scalar>, TSoScalar>::value;
  }

  /// This function determines if the type of this simulation object is the same
  /// as `object` without taking the backend into account.
  /// @param object simulation object can have any backend
  template <typename TSo>
  static constexpr bool IsSoType(const TSo *object) {
    using Type = std::decay_t<std::remove_pointer_t<decltype(object)>>;
    using ScalarType = typename Type::template Self<Scalar>;
    return std::is_same<MostDerived<Scalar>, ScalarType>::value;
  }

  /// Casts this to a simulation object of type `TSo` with the current `Backend`
  /// This function is used to simulate if constexpr functionality and won't be
  /// needed after we swith to C++17
  /// @tparam TSo target simulaton object type with any backend
  template <typename TSo>
  constexpr auto &&ReinterpretCast() {
    using TargetType = typename TSo::template Self<Backend>;
    return reinterpret_cast<TargetType &&>(*this);
  }

  /// Casts this to a simulation object of type `TSo` with the current `Backend`
  /// This function is used to simulate if constexpr functionality and won't be
  /// needed after we swith to C++17
  /// @tparam TSo target simulaton object type with any backend
  template <typename TSo>
  constexpr const auto *ReinterpretCast() const {
    using TargetType = typename TSo::template Self<Backend>;
    return reinterpret_cast<const TargetType *>(this);
  }

  /// Casts this to a simulation object of type `TSo` with the current `Backend`
  /// This function is used to simulate if constexpr functionality and won't be
  /// needed after we swith to C++17
  /// @tparam TSo target simulaton object type with any backend
  template <typename TSo>
  constexpr auto &&ReinterpretCast(const TSo *object) {
    using TargetType = typename TSo::template Self<Backend>;
    return reinterpret_cast<TargetType &&>(*this);
  }

  MostDerived<Backend> *operator->() {
    return static_cast<MostDerived<Backend> *>(this);
  }

  const MostDerived<Backend> *operator->() const {
    return static_cast<const MostDerived<Backend> *>(this);
  }

  void RunDiscretization() {}

  SoUid GetUid() const { return uid_[kIdx]; }

  uint32_t GetBoxIdx() const { return box_idx_[kIdx]; }

  void SetBoxIdx(uint32_t idx) { box_idx_[kIdx] = idx; }

  // TODO(ahmad) this only works for SOA backend add check for SOA
  // used only for cuda and opencl code
  uint32_t *GetBoxIdPtr() { return box_idx_.data(); }

  /// Return simulation object pointer
  MostDerivedSoPtr GetSoPtr() const { return MostDerivedSoPtr(uid_[kIdx]); }

  void SetNumaNode(typename SoHandle::NumaNode_t numa_node) {
    numa_node_[kIdx] = numa_node;
  }

  template <typename TResourceManager = ResourceManager<>>
  SoHandle GetSoHandle() const {
    auto type_idx =
        TResourceManager::template GetTypeIndex<MostDerivedScalar>();
    return SoHandle(numa_node_[kIdx], type_idx, Base::GetElementIdx());
  }

  // ---------------------------------------------------------------------------
  // Biology modules
  using BiologyModules =
      typename TCompileTimeParam::template CTMap<MostDerivedScalar,
                                                 0>::BiologyModules::Variant_t;

  /// Add a biology module to this cell
  /// @tparam TBiologyModule type of the biology module. Must be in the set of
  ///         types specified in `BiologyModules`
  template <typename TBiologyModule>
  void AddBiologyModule(TBiologyModule &&module) {
    biology_modules_[kIdx].emplace_back(module);
  }

  /// Remove a biology module from this cell
  template <typename TBiologyModule>
  void RemoveBiologyModule(const TBiologyModule *remove_module) {
    for (unsigned int i = 0; i < biology_modules_[kIdx].size(); i++) {
      const TBiologyModule *module =
          get_if<TBiologyModule>(&biology_modules_[kIdx][i]);
      if (module == remove_module) {
        biology_modules_[kIdx].erase(biology_modules_[kIdx].begin() + i);
        // if remove_module was before or at the current run_bm_loop_idx_[kidx],
        // correct it by subtracting one.
        run_bm_loop_idx_[kIdx] -= i > run_bm_loop_idx_[kIdx] ? 0 : 1;
      }
    }
  }

  /// Execute all biology modulesq
  void RunBiologyModules() {
    RunVisitor<MostDerived<Backend>> visitor(
        static_cast<MostDerived<Backend> *>(this));
    for (run_bm_loop_idx_[kIdx] = 0;
         run_bm_loop_idx_[kIdx] < biology_modules_[kIdx].size();
         ++run_bm_loop_idx_[kIdx]) {
      auto &module = biology_modules_[kIdx][run_bm_loop_idx_[kIdx]];
      visit(visitor, module);
    }
  }

  /// Get all biology modules of this cell that match the given type.
  /// @tparam TBiologyModule  type of the biology module
  template <typename TBiologyModule>
  std::vector<const TBiologyModule *> GetBiologyModules() const {
    std::vector<const TBiologyModule *> modules;
    for (unsigned int i = 0; i < biology_modules_[kIdx].size(); i++) {
      const TBiologyModule *module =
          get_if<TBiologyModule>(&biology_modules_[kIdx][i]);
      if (module != nullptr) {
        modules.push_back(module);
      }
    }
    return modules;
  }

  /// Return all biology modules
  const auto &GetAllBiologyModules() const { return biology_modules_[kIdx]; }
  // ---------------------------------------------------------------------------

  void RemoveFromSimulation() const {
    Simulation_t::GetActive()->GetExecutionContext()->RemoveFromSimulation(
        uid_[kIdx]);
  }

  template <typename TEvent, typename TOther>
  void EventHandler(const TEvent &event, TOther *other) {
    // call event handler for biology modules
    auto *other_bms = &(other->biology_modules_[other->kIdx]);
    BiologyModuleEventHandler(event, &(biology_modules_[kIdx]), other_bms);
  }

  template <typename TEvent, typename TLeft, typename TRight>
  void EventHandler(const TEvent &event, TLeft *left, TRight *right) {
    // call event handler for biology modules
    auto *left_bms = &(left->biology_modules_[left->kIdx]);
    auto *right_bms = &(right->biology_modules_[right->kIdx]);
    BiologyModuleEventHandler(event, &(biology_modules_[kIdx]), left_bms,
                              right_bms);
  }

 protected:
  /// unique id
  vec<SoUid> uid_ = {{}};
  /// Grid box index
  vec<uint32_t> box_idx_ = {{}};
  /// collection of biology modules which define the internal behavior
  vec<std::vector<BiologyModules>> biology_modules_;

 private:
  /// Helper variable used to support removal of biology modules while
  /// `RunBiologyModules` iterates over them.
  /// Due to problems with restoring this member for the SOA data layout, it is
  /// not ignored for ROOT I/O.
  vec<uint32_t> run_bm_loop_idx_ = {{0}};

  // TODO documentation
  vec<typename SoHandle::NumaNode_t> numa_node_ = {{}};


  /// @brief Function to copy biology modules from one structure to another
  /// @param event event will be passed on to biology module to determine
  ///        whether it should be copied to destination
  /// @param src  source vector of biology modules
  /// @param dest destination vector of biology modules
  /// @tparam TBiologyModules std::vector<Variant<[list of biology modules]>>
  template <typename TEvent, typename TSrcBms, typename TDestBms>
  void CopyBiologyModules(const TEvent &event, TSrcBms *src, TDestBms *dest) {
    auto copy = [&](auto &bm) {
      if (bm.Copy(event.kEventId)) {
        raw_type<decltype(bm)> new_bm(event, &bm);
        dest->emplace_back(std::move(new_bm));
      }
    };
    for (auto &module : *src) {
      visit(copy, module);
    }
  }

  /// @brief Function to invoke the EventHandler of the biology module or remove
  ///                  it from `current`.
  /// Forwards the event handler call to each biology modules of the triggered
  /// simulation object and removes biology modules if they are flagged.
  template <typename TEvent, typename TBiologyModules1,
            typename... TBiologyModules>
  void BiologyModuleEventHandler(const TEvent &event, TBiologyModules1 *current,
                                 TBiologyModules *... bms) {
    // call event handler for biology modules
    uint64_t cnt = 0;
    auto call_bm_event_handler = [&](auto &bm) {
      using BiologyModuleType = raw_type<decltype(bm)>;

      /// return nullptr of condition is false or pointer to object in variant
      auto extract = [](bool condition, auto *variant) -> BiologyModuleType * {
        if (condition) {
          return get_if<BiologyModuleType>(variant);
        }
        return nullptr;
      };

      if (!bm.Remove(event.kEventId)) {
        bool copy = bm.Copy(event.kEventId);
        bm.EventHandler(event, extract(copy, &((*bms)[cnt]))...);
        cnt += copy ? 1 : 0;
      }
    };
    for (auto &el : *current) {
      visit(call_bm_event_handler, el);
    }

    // remove biology modules from current
    bool remove;
    auto remove_from_current = [&](auto &bm) {
      remove = bm.Remove(event.kEventId);
    };
    for (auto it = current->begin(); it != current->end();) {
      remove = false;
      visit(remove_from_current, *it);
      if (remove) {
        it = current->erase(it);
      } else {
        ++it;
      }
    }
  }
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
