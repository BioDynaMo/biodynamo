#ifndef BASE_SIMULATION_TEST_H_
#define BASE_SIMULATION_TEST_H_

#include <string>
#include <vector>

#include <TROOT.h>
#include <TFile.h>
#include <TClass.h>

#include "gtest/gtest.h"

#include "simulation/ecm.h"
#include "physics/physical_node.h"

namespace bdm {

using std::string;
using simulation::ECM;

/**
 * This is the base class for all simulation tests. Only the simulation code has to be written in the subclass.
 * <code>BaseSimulationTest</code> takes care of setting up the environment, comparing it to the expected value
 * as well as documentation of test runtime. At the moment it just measures execution time. This means that all tests
 * should run on the same machine with the same load profile.
 *
 * If the code for the simulate state changes or if new tests are added one wants to update the reference files for
 * the simulation. This can easily be done by setting the command line parameter<code>--update-references</code>
 * e.g. ./runBiodynamoTests --update-references
 * In this case no assertions are made
 *
 * Assertions can be turned off using the parameter <code>--disable-assertions</code>
 *
 * Each SimulationTest has two associated files in the test resource folder
 *  ClassName.json      contains the reference simulate state in json format
 *  ClassName.csv       contains the runtimes for each git commit in the following format: git_commit_id;runtime_in_ms
 */
class BaseSimulationTest : public ::testing::Test {
 public:
  /**
   * This parameters are set in the main function if the command line parameter is specified
   */
  static bool update_sim_state_reference_file_;
  static bool disable_assertions_;

  BaseSimulationTest();

  virtual ~BaseSimulationTest();

  /**
   * this function sets up BioDynaMo and ensures that simulations don't influence each other
   */
  virtual void SetUp();

  virtual void TearDown();

  /**
   * method that will be called from the test
   * executes the simulation, keeps track of the runtime and asserts the simulation state
   */
  void run();

 protected:
  /**
   * holds instances additional instances of PhysicalNode that are created during the simulation
   * and are not an instanceof PhysicalCylinder and PhysicalSphere
   */
  std::vector<physics::PhysicalNode::UPtr> physical_nodes_;

  /**
   * Simulation logic defined in the subclass
   */
  virtual void simulate() = 0;

  /**
   * Subclass has to provide the test name. BaseSimulationTest needs that to access test resources like the
   * reference simulation state and the runtime file
   */
  virtual string getTestName() const = 0;

  /**
   * helper function used to replicate the initialization point of PhysicalNodeMovementListener
   */
  void initPhysicalNodeMovementListener();

 private:
  long runtime_ = 0;

  /**
   * this function ensures that the simulation state is equal to the reference one stored in TestCaseName.json
   */
  void assertSimulationState();

  /**
   * This function updates the runtime for the last git commit
   */
  void persistRuntime();

  /**
   * returns the current time stamp in milliseconds
   */
  long timestamp();

  /**
   * helper function to read the contents of a file into the param content
   */
  void readFromFile(const string& file_path, string& content);

  /**
   * helper function to write an object into a file
   */
  template<class T>
  void writeToFile(const string& file_path, T& content);

  /**
   * helper function used to execute a system command and retrieve its output
   * https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
   */
  string exec(const char* cmd);
};

}  // namespace bdm

#endif  // BASE_SIMULATION_TEST_H_
