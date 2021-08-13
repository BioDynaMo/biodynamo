#ifndef MFEM_MOL_H_
#define MFEM_MOL_H_

#include "mfem.hpp"

namespace bdm {
namespace experimental {

class MethodOfLineSolver {
 protected:
  /// The underlying mesh on which we solve the PDE
  mfem::Mesh* mesh_;
  /// The ODE solver used to integrate in time.
  mfem::ODESolver* ode_solver_;
  /// Arbitrary order H1-conforming (continuous) finite elements. (quote MFEM)
  mfem::H1_FECollection fe_coll_;
  /// Class FiniteElementSpace - responsible for providing FEM view of the
  /// mesh, mainly managing the set of degrees of freedom. (quote MFEM)
  mfem::FiniteElementSpace fespace_;
  /// The solution computed from the PDE.
  mfem::GridFunction u_gf_;
  /// Vector representation of the PDE solution.
  mfem::Vector u_;

  /// Internally used function for initilization.
  void Initialize();
  /// Function used to set the boundaries conditions.
  void SetBoundaryConditions();

 public:
  MethodOfLineSolver(int order, int dimension);
  ~MethodOfLineSolver();
  // No copy (assignment) as of now.
  MethodOfLineSolver(const MethodOfLineSolver&) = delete;
  MethodOfLineSolver& operator=(const MethodOfLineSolver&) = delete;
  MethodOfLineSolver(MethodOfLineSolver&&) = delete;
  MethodOfLineSolver& operator=(MethodOfLineSolver&&) = delete;

  /// Execute one ODE timestep `dt`, e.g. compute `u(t+dt)` from `u(t)`.
  void Step(double dt);

  /// Export the continuum model to paraview.
  void Visualize();

  /// Get the value of the GridFunction solution at a certain position.
  double GetSolutionAtPosition();
};

}  // namespace experimental
}  // namespace bdm

#endif  // MFEM_MOL_H_
