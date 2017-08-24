#ifndef COMPILE_TIME_PARAM_H_
#define COMPILE_TIME_PARAM_H_

#include "backend.h"
#include "biology_module_util.h"
#include "cell.h"
#include "variadic_template_parameter_util.h"
#include "variant.h"

namespace bdm {

/// \brief Defines default compile time parameters
/// Values can be overwritten by subclassing it.
/// `struct bdm::CompileTimeParam` has been forward declared by classes using
/// compile time parameters. This struct must be defined -- e.g. by using
/// `BDM_DEFAULT_COMPILE_TIME_PARAM()`
/// @tparam TOptional DefaultCompileTimeParam must be a template class, since
///         Types used inside are not fully defined yet. Therefore, a
///         meaningless template parameter has been introduced.
template <size_t TOptional = 1>
struct DefaultCompileTimeParam {
  using Backend = Soa;
  using BiologyModules = Variant<NullBiologyModule>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

/// Macro which sets compile time parameter to DefaultCompileTimeParam
/// Caution: This call must be made from namespace `::bdm`. Otherwise,
/// the forward declared `struct bdm::CompileTimeParam` will not be defined.
#define BDM_DEFAULT_COMPILE_TIME_PARAM() \
  struct CompileTimeParam : public DefaultCompileTimeParam<> {};

}  // namespace bdm

#endif  // COMPILE_TIME_PARAM_H_
