#include "unit/separate_binary/catalyst_adaptor_test.h"

namespace bdm {

/// Tests if simulation_info.json is generated correctly during initialization.
TEST_F(CatalystAdaptorTest, GenerateSimulationInfoJson) {
  auto* param = simulation_->GetParam();

  // set-up Param values
  param->export_visualization_ = true;
  param->visualize_sim_objects_.clear();
  param->visualize_sim_objects_["cell"] = {};
  param->visualize_sim_objects_["neurite"] = {};
  param->visualize_diffusion_.clear();
  param->visualize_diffusion_.push_back({"sodium", true, true});

  std::unordered_map<std::string, Shape> shapes;
  shapes["cell"] = kSphere;
  shapes["neurite"] = kCylinder;

  CatalystAdaptor<>::GenerateSimulationInfoJson(shapes);

  ASSERT_TRUE(FileExists(kSimulationInfoJson));

  // check file contents
  std::ifstream ifs(kSimulationInfoJson);
  std::stringstream buffer;
  buffer << ifs.rdbuf();

  const char* expected = R"STR({
  "simulation": {
    "name":"MySimulation",
    "result_dir":"output/MySimulation"
  },
  "sim_objects": [
    {
      "name":"neurite",
      "glyph":"BDMGlyph",
      "shape":"Cylinder",
      "x_scaling_attribute":"diameter_",
      "y_scaling_attribute":"actual_length_",
      "z_scaling_attribute":"diameter_",
      "Vectors":"spring_axis_",
      "MassLocation":"mass_location_"
    },
    {
      "name":"cell",
      "glyph":"Glyph",
      "shape":"Sphere",
      "scaling_attribute":"diameter_"
    }
  ],
  "extracellular_substances": [
    { "name":"sodium", "has_gradient":"true" }
  ]
}
)STR";

  EXPECT_EQ(expected, buffer.str());
}

/// Tests if the catalyst state is generated.
TEST_F(CatalystAdaptorTest, GenerateParaviewState) {
  // before we can call finalize we need to modify the json object
  // we need to remove entries for sim_objects and extracellular_substances
  // because there are no corresponding data files available.
  // Therefore the script would fail.
  const char* empty_json = R"STR({
  "simulation": {
    "name":"MySimulation",
    "result_dir":"output/MySimulation"
  },
  "sim_objects": [],
  "extracellular_substances": []
}
)STR";

  std::ofstream ofs;
  ofs.open(kSimulationInfoJson);
  ofs << empty_json;
  ofs.close();

  CatalystAdaptor<>::GenerateParaviewState();

  ASSERT_TRUE(FileExists(kParaviewState));
}

/// Test if the objects that we want to output for visualization are indeed
/// the only ones (no more, no less).
TEST_F(CatalystAdaptorTest, CheckVisualizationSelection) {
  Simulation<> sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();

  enum Substances { kSubstance0, kSubstance1, kSubstance2 };

  // Create two types of cells
  auto my_cell = rm->New<MyCell>();
  auto cell = rm->New<MyCell>();
  auto neuron = rm->New<MyNeuron>();

  // Define the substances
  ModelInitializer::DefineSubstance(kSubstance0, "Substance_0", 0.5, 0);
  ModelInitializer::DefineSubstance(kSubstance2, "Substance_2", 0.5, 0);
  ModelInitializer::DefineSubstance(kSubstance1, "Substance_1", 0.5, 0);

  int l = -100;
  int r = 100;
  rm->GetDiffusionGrids()[kSubstance0]->Initialize({l, r, l, r, l, r});
  rm->GetDiffusionGrids()[kSubstance1]->Initialize({l, r, l, r, l, r});
  rm->GetDiffusionGrids()[kSubstance2]->Initialize({l, r, l, r, l, r});

  auto* param = sim.GetParam();
  param->export_visualization_ = true;

  // We selection Substance_1 for export
  Param::VisualizeDiffusion vd;
  vd.name_ = "Substance_1";
  param->visualize_diffusion_.push_back(vd);

  // We select MyCell for export
  param->visualize_sim_objects_["MyCell"] = {};
  param->visualize_sim_objects_["Cell"] = {};

  // Write diffusion visualization to file
  CatalystAdaptor<> adaptor("");
  adaptor.Visualize(1, true);
  adaptor.WriteToFile(0);

  // Read back from file
  std::vector<std::string> needed_files;
  auto filename1 = Concat(sim.GetOutputDir(), "/Substance_1-0_0.vti");
  auto filename2 = Concat(sim.GetOutputDir(), "/Substance_1-0.pvti");
  auto filename3 = Concat(sim.GetOutputDir(), "/MyCell-0.pvtu");
  auto filename4 = Concat(sim.GetOutputDir(), "/Cell-0.pvtu");
  needed_files.push_back(filename1);
  needed_files.push_back(filename2);
  needed_files.push_back(filename3);
  needed_files.push_back(filename4);

  for (auto& file : needed_files) {
    if (!FileExists(file.c_str())) {
      std::cout << file << " was not generated!" << std::endl;
      FAIL();
    }
  }

  std::vector<std::string> not_needed_files;
  filename1 = Concat(sim.GetOutputDir(), "/Substance_0-0_0.vti");
  filename2 = Concat(sim.GetOutputDir(), "/Substance_2-0_0.vti");
  filename3 = Concat(sim.GetOutputDir(), "/MyNeuron-0.pvtu");
  not_needed_files.push_back(filename1);
  not_needed_files.push_back(filename2);
  not_needed_files.push_back(filename3);

  for (auto& file : not_needed_files) {
    if (FileExists(file.c_str())) {
      std::cout << file << " was generated, but shouldn't have!" << std::endl;
      FAIL();
    }
  }
}

}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
