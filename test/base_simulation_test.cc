#include "base_simulation_test.h"

#include <fstream>
#include <chrono>
#include <iostream>

#include "json/json.h"

#include "physics/default_force.h"
#include "physics/physical_node_movement_listener.h"

namespace bdm {

using std::ofstream;
using cells::Cell;
using local_biology::CellElement;
using physics::PhysicalNode;
using physics::DefaultForce;
using physics::PhysicalObject;
using physics::PhysicalNodeMovementListener;
using spatial_organization::SpaceNode;
using synapse::Excrescence;

bool BaseSimulationTest::update_sim_state_reference_file_ = false;
bool BaseSimulationTest::disable_assertions_ = false;

BaseSimulationTest::BaseSimulationTest() {
}

BaseSimulationTest::~BaseSimulationTest() {
}

void BaseSimulationTest::SetUp() {
  ECM::getInstance()->clearAll();
  Cell::reset();
  CellElement::reset();
  PhysicalNode::reset();
  SpaceNode < PhysicalNode > ::reset();

  Random::setSeed(1L);

  PhysicalObject::setInterObjectForce(DefaultForce::UPtr(new DefaultForce()));
}

void BaseSimulationTest::TearDown() {
}

void BaseSimulationTest::run() {
  // run simulation
  long start = timestamp();
  simulate();
  long end = timestamp();
  runtime_ = end - start;
  std::cout << "RUNTIME " << getTestName() << " " << (end-start) << std::endl;
  // ensure correct result
  if (!disable_assertions_) {
    assertSimulationState();
  }
}

void BaseSimulationTest::assertSimulationState() {
  // create Json string of simulation state
  StringBuilder sb;
  ECM::getInstance()->simStateToJson(sb);

  string reference_file_name = "test_resources/" + getTestName() + ".json";

  if (update_sim_state_reference_file_) {
    // update reference file
    std::cout << "NOTE: parameter --update-references specified: Reference File will be updated and "
        "no assertions will be performed" << std::endl;
    string content = sb.str();
    writeToFile("../test/resources/" + getTestName() + ".json", content);
    return;
  }

  // read in reference simulation state
  string expected_string;
  readFromFile(reference_file_name, expected_string);

  // create Json objects
  Json::Reader reader;
  Json::Value actual;
  Json::Value expected;
  Json::Value::epsilon_ = 1e-10;
  reader.parse(sb.str(), actual);
  reader.parse(expected_string, expected);

  // compare Json objects for equality
  bool equal = expected == actual;
  EXPECT_TRUE(equal);
  if (equal) {
    persistRuntime();
  } else {
    writeToFile("failed_" + getTestName() + "_actual", actual);
    writeToFile("failed_" + getTestName() + "_expected", expected);
  }
}

void BaseSimulationTest::persistRuntime() {
  // get last commit
  auto last_commit = exec("git rev-parse HEAD").substr(0, 40);

  // read in csv file with runtimes
  string filename = "../test/resources/" + getTestName() + ".csv";
  string runtime_csv;
  readFromFile(filename, runtime_csv);

  // if entry with this commit exists: remove it;
  auto found = runtime_csv.find(last_commit);
  if (found != string::npos) {
    runtime_csv = runtime_csv.substr(0, found);
  }
  // append new runtime
  stringstream ss;
  ss << last_commit << ';' << runtime_ << std::endl;  // convert runtime_ to seconds
  runtime_csv += ss.str();

  writeToFile(filename, runtime_csv);
}

long BaseSimulationTest::timestamp() {
  namespace sc = std::chrono;
  auto time = sc::system_clock::now();
  auto since_epoch = time.time_since_epoch();
  auto millis = sc::duration_cast < sc::milliseconds > (since_epoch);
  return millis.count();
}

void BaseSimulationTest::readFromFile(const string &file_path, string &content) {
  std::ifstream ifs(file_path);
  content.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

template<class T>
void BaseSimulationTest::writeToFile(const string& file_path, T& content) {
  ofstream ofs(file_path);
  ofs << content;
}

string BaseSimulationTest::exec(const char* cmd) {
  char buffer[128];
  string result = "";
  std::shared_ptr < FILE > pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (!feof(pipe.get())) {
    if (fgets(buffer, 128, pipe.get()) != NULL) {
      result += buffer;
    }
  }
  return result;
}

void BaseSimulationTest::initPhysicalNodeMovementListener() {
  PhysicalNodeMovementListener::setMovementOperationId((int) (10000 * Random::nextDouble()));
}

}  // namespace bdm
