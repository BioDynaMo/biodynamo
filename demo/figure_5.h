#ifndef INTEGRATION_FIGURE_5_
#define INTEGRATION_FIGURE_5_

#include <unistd.h>
#include "biodynamo.h"
#include "neuroscience/neuroscience.h"

namespace bdm {

using neuroscience::Neuron;
using neuroscience::Neurite;

template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend>,
                          public neuroscience::DefaultCompileTimeParam<TBackend> {
  // using BiologyModules = Variant<TestBehaviour>;
  using AtomicTypes = VariadicTypedef<Cell, Neuron, Neurite>;
};

inline int Simulate(int argc, const char** argv) {
  InitializeBioDynamo(argc, argv);
  gErrorIgnoreLevel = kWarning;  // TODO make command line argument

  // hard code parameters to avoid parameter file
  Param::live_visualization_ = true;

  Scheduler<> scheduler;

  // creating a first cell, with a neurite going straight up.
  //    creating a 4-uple Cell-SomaElement-PhysicalSphere-SpatialOrganizerNode
  auto neuron = Rm()->New<Neuron>();
  neuron.SetPosition({ 0, 0, -100 });
  neuron.SetMass(1);
  neuron.SetDiameter(10);
  // TODO neuron set color to solid red

  //    creating a single neurite
  auto ne = neuron.ExtendNewNeurite(2.0, 0, 0).Get();
  // auto ne = neuron.ExtendNewNeurite(2.0, Math::kPi / 4.0, Math::kPi / 2.0).Get();
  //    elongating the neurite :
  // std::array<double, 3> direction_up = { 0, 1, 1 };
  std::array<double, 3> direction_up = { 0, 0, 1 };
  for (int i = 0; i < 103; i++) {
    ne.ElongateTerminalEnd(300, direction_up);
    ne.RunDiscretization();
    scheduler.Simulate(1);
  }

  // 3) creating three additional spheres:
  auto cell_b = Rm()->New<Cell>();
  cell_b.SetPosition({ 10, 0, 0 });
  cell_b.SetMass(3);
  cell_b.SetDiameter(10);
  // TODO cell_b->SetColor(Param::kYellowSolid);

  // auto cell_c = Rm()->New<Cell>();
  // cell_c.SetPosition({ -10, 0, 100 });
  // cell_c.SetMass(3);
  // cell_c.SetDiameter(10);
  // // TODO cell_c->SetColor(Param::kYellowSolid);
  //
  // auto cell_d = Rm()->New<Cell>();
  // cell_d.SetPosition({ 10, 0, 160});
  // cell_d.SetMass(2);
  // cell_d.SetDiameter(10);
  // // TODO cell_d->SetColor(Param::kYellowSolid);

  // 4) setting a large diameter OR letting them grow
  for (int i = 0; i < 15; i++) {
    cell_b.SetDiameter(cell_b.GetDiameter()+3);
    // cell_c.SetDiameter(cell_c.GetDiameter()+2);
    // cell_d.SetDiameter(cell_d.GetDiameter()+1);
    scheduler.Simulate(1);
  }

  scheduler.Simulate(1000);

  // FIXME check result
  return 0;
}

}  // namespace bdm

#endif  // INTEGRATION_FIGURE_5_
