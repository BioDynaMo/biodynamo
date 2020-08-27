#include "core/operation/bound_space_op.h"
#include "core/operation/diffusion_op.h"
#include "core/operation/displacement_op.h"
#include "core/operation/displacement_op_cuda.h"
#include "core/operation/displacement_op_opencl.h"
#include "core/operation/dividing_cell_op.h"
#include "core/operation/operation.h"

namespace bdm {

BDM_REGISTER_OP(BoundSpace, "bound space", kCpu);

BDM_REGISTER_OP(DiffusionOp, "diffusion", kCpu);

BDM_REGISTER_OP(DisplacementOp, "displacement", kCpu);

#ifdef USE_CUDA
BDM_REGISTER_OP(DisplacementOpCuda, "displacement", kCuda);
#endif

BDM_REGISTER_OP(DividingCellOp, "DividingCellOp", kCpu);

#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
BDM_REGISTER_OP(DisplacementOpOpenCL, "displacement", kOpenCl);
#endif

}  // namespace bdm
