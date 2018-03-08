#include "backend.h"
#include "cell.h"
#include "neuroscience/compile_time_param.h"
#include "neuroscience/neurite.h"
#include "neuroscience/neuron.h"
#include "unit/test_util.h"
#include "gtest/gtest.h"
#include "biodynamo.h"

// just cc is good - h file for new types
// got back to hiim for compile issues

namespace bdm {

  using neuroscience::Neurite;
  using neuroscience::Neuron;
  
  template <typename TBackend>
  struct CompileTimeParam : public DefaultCompileTimeParam<TBackend>, public neuroscience::DefaultCompileTimeParam<TBackend> {
    using AtomicTypes = VariadicTypedef<Cell, Neuron, Neurite>;
  };

//  TEST(DisplacementOpTest, ComputeTests) { RunTest(); }
  
  TEST(MechanicalInteraction, StraightCylinderGrowth) {
    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, -100 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    std::cout << "cell created" << std::endl;

    auto ne = neuron.ExtendNewNeurite(0, 0, 1).Get(); // .Get();?

    std::cout << "neurite created" << std::endl;
    
    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
//    Scheduler<> scheduler;

    std::cout << "grid initialized" << std::endl;

    std::array<double, 3> direction_up = {0, 0, 1};
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(100, direction_up);
      ne.RunDiscretization();
//      scheduler.Simulate(1);
    }
    
    std::cout << "neurite elongation done" << std::endl;

/*
    auto daughters=neuron.GetDaughters();
    Neurite daughter;
    std::array<double, 3> axis;

    for (int i = 0; i < daughters.size(); ++i) {
      daughter=daughters[i];
      axis=daughter.GetSpringAxis();

      EXPECT_NEAR(axis[2], 1, abs_error<double>::value);
    }
*/

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    std::cout << "Spring axis acquired" << std::endl;
    
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);
    
//    Neurite daughter = ne.getDaughterLeft();
//    std::array<double, 3> daughterAxis = daughter.GetSpringAxis();
    
//    EXPECT_NEAR(daughterAxis[2], 1, abs_error<double>::value);
    
  }
   
  
} // end namespace bdm
