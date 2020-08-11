#include "core/operation/bound_space_op.h"
#include "core/operation/diffusion_op.h"
#include "core/operation/displacement_op.h"
#include "core/operation/displacement_op_cuda.h"
#include "core/operation/displacement_op_opencl.h"
#include "core/operation/dividing_cell_op.h"
#include "core/operation/operation.h"

namespace bdm {

REGISTER_OP(BoundSpace, "bound space", kCpu);

REGISTER_OP(DiffusionOp, "diffusion", kCpu);

REGISTER_OP(DisplacementOp, "displacement", kCpu);

#ifdef USE_CUDA
REGISTER_OP(DisplacementOpCuda, "displacement", kCuda);
#endif

REGISTER_OP(DividingCellOp, "DividingCellOp", kCpu);

#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
REGISTER_OP(DisplacementOpOpenCL, "displacement", kOpenCl);
#endif

}  // namespace bdm
