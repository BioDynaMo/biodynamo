#include "mfem_mol.h"

namespace bdm {
namespace experimental {

MethodOfLineSolver::MethodOfLineSolver(int order, int dimension)
    : fe_coll_(order, dimension) {
  mesh_ = nullptr;
  ode_solver_ = nullptr;
}
MethodOfLineSolver::~MethodOfLineSolver() {
  delete mesh_;
  delete ode_solver_;
}

void MethodOfLineSolver::Initialize() { return; }

void MethodOfLineSolver::SetBoundaryConditions() { return; }

void MethodOfLineSolver::Step(double dt) { return; }

void MethodOfLineSolver::Visualize() { return; }

double MethodOfLineSolver::GetSolutionAtPosition() { return 0.0; }

}  // namespace experimental
}  // namespace bdm
