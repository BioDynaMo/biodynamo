#ifndef UNIT_DEFAULT_CTPARAM_H_
#define UNIT_DEFAULT_CTPARAM_H_

#include "compile_time_param.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend> {};

}  // namespace bdm

#endif  // UNIT_DEFAULT_CTPARAM_H_
