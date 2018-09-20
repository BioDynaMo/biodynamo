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

#ifndef COMPILE_TIME_PARAM_H_
#define COMPILE_TIME_PARAM_H_

#include <cpp_magic.h>

#include "backend.h"
#include "biology_module_util.h"
#include "cell.h"
#include "compile_time_list.h"
#include "variant.h"

namespace bdm {

#define BDM_CTPARAM_BASES_ITERATOR(module) \
  , public module::DefaultCTParam<Backend>

#define BDM_CTPARAM_BASES(...) \
  EVAL(LOOP(BDM_CTPARAM_BASES_ITERATOR, __VA_ARGS__))

#define BDM_CTPARAM_HEADER_CTMAP_BASES_ITERATOR(module) \
  , public module::DefaultCTParam<Backend>::template CTMap<TSimObject, Dummy>

#define BDM_CTPARAM_HEADER_CTMAP_BASES(...) \
  EVAL(LOOP(BDM_CTPARAM_HEADER_CTMAP_BASES_ITERATOR, __VA_ARGS__))

#define BDM_CTPARAM_HEADER_PARAM_BASES_ITERATOR(module) , public module::Param

#define BDM_CTPARAM_HEADER_PARAM_BASES(...) \
  EVAL(LOOP(BDM_CTPARAM_HEADER_PARAM_BASES_ITERATOR, __VA_ARGS__))

#define BDM_CTPARAM_HEADER_PARAM_LOAD_BODY_ITERATOR(module) \
  module::Param::AssignFromConfig(config);

#define BDM_CTPARAM_HEADER_PARAM_LOAD_BODY(...) \
  EVAL(LOOP(BDM_CTPARAM_HEADER_PARAM_LOAD_BODY_ITERATOR, __VA_ARGS__))

// -----------------------------------------------------------------------------

/// Macro to define `bdm::CompileTimeParam`\n
/// Takes a list of biodynamo modules as argument (optional).
#define BDM_CTPARAM(...)                                        \
  template <typename Backend>                                   \
  struct CompileTimeParam : public bdm::DefaultCTParam<Backend> \
                                BDM_CTPARAM_BASES(__VA_ARGS__)

/// Macro to insert boilerplate code into bdm::CompileTimeParam.\n
/// Parameter like in  `BDM_CTPARAM`
#define BDM_CTPARAM_HEADER(...)                                            \
  template <typename TSimObject, int Dummy>                                \
  struct CTMap                                                             \
      : public DefaultCTParam<Backend>::template CTMap<TSimObject, Dummy>  \
        BDM_CTPARAM_HEADER_CTMAP_BASES(__VA_ARGS__) {};                    \
                                                                           \
  struct Param : public ::bdm::Param BDM_CTPARAM_HEADER_PARAM_BASES(       \
                     __VA_ARGS__) {                                        \
    void AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) { \
      ::bdm::Param::AssignFromConfig(config);                              \
      BDM_CTPARAM_HEADER_PARAM_LOAD_BODY(__VA_ARGS__)                      \
    }                                                                      \
  };

/// Macro to define the default parameters for a specific simulation object.\n
#define BDM_DEFAULT_CTPARAM_FOR(sim_object) \
  template <int Dummy>                      \
  struct CTMap<sim_object, Dummy> : public CTMapDefault<sim_object, Dummy>

/// Macro to overwrite the default parameter of a specific simulation objects.
#define BDM_CTPARAM_FOR(module, sim_object)                     \
  template <int Dummy>                                          \
  struct CTMap<module::sim_object, Dummy>                       \
      : public module::DefaultCTParam<Backend>::template CTMap< \
            module::sim_object, Dummy>

/// Define global default compile time parameter for simulation objects
template <typename TSimObject, int Dummy>
struct CTMapDefault {
  using BiologyModules = CTList<NullBiologyModule>;
};

/// \brief Defines default compile time parameters for the BioDynaMo core.
/// Values can be overwritten by subclassing it.
/// `struct bdm::CompileTimeParam` has been forward declared by classes using
/// compile time parameters. This struct must be defined -- e.g. by using
/// `BDM_DEFAULT_COMPILE_TIME_PARAM()`
/// NB Can't be used in tests because CompileTimeParam is hardcoded in Self
/// alias
/// @tparam TBackend required to use simulation objects with different Backend
template <typename TBackend = Soa>
struct DefaultCTParam {
  template <typename TTBackend>
  using Self = CompileTimeParam<TTBackend>;
  using Backend = TBackend;

  /// Defines backend used in ResourceManager
  using SimulationBackend = Soa;
  using SimObjectTypes = CTList<Cell>;

  // must be empty. otherwise ambigous with other specializations
  template <typename TSimObject, int Dummy>
  struct CTMap {};

  BDM_DEFAULT_CTPARAM_FOR(Cell){};
};

}  // namespace bdm

#endif  // COMPILE_TIME_PARAM_H_
