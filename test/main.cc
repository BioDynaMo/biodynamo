#include <string>

// #include <TROOT.h>
// #include <TApplication.h>
// #include <TSystem.h>
// #include <TBenchmark.h>

#include "version.h"
#include "gtest/gtest.h"

#include "base_simulation_test.h"



using std::string;

int main(int argc, char **argv) {
  //  // Setup ROOT
  //  gROOT->SetBatch();
  //  TApplication theApp("biotest", &argc, argv);
  //
  // // manual processing for additional parameters
  // string update_references = "--update-references";
  // string disable_assertions = "--disable-assertions";
  // for (int i = 0; i < argc; i++) {
  //   if (!update_references.compare(argv[i])) {
  //     bdm::BaseSimulationTest::update_sim_state_reference_file_ = true;
  //   } else if (!disable_assertions.compare(argv[i])) {
  //     bdm::BaseSimulationTest::disable_assertions_ = true;
  //   }
  // }

  ::testing::InitGoogleTest(&argc, argv);
  // gBenchmark = new TBenchmark();
  // gBenchmark->Start("biotest");
  int ret = RUN_ALL_TESTS();
  // gBenchmark->Stop("biotest");
  //
  // // print system info
  // Bool_t unix = strcmp(gSystem->GetName(), "Unix") == 0;
  // printf("******************************************************************\n");
  // if (unix) {
  //    TString sp = gSystem->GetFromPipe("uname -a");
  //    sp.Resize(60);
  //    printf("*  SYS: %s\n", sp.Data());
  //    if (strstr(gSystem->GetBuildNode(), "Linux")) {
  //       sp = gSystem->GetFromPipe("lsb_release -d -s");
  //       printf("*  SYS: %s\n", sp.Data());
  //    }
  //    if (strstr(gSystem->GetBuildNode(), "Darwin")) {
  //       sp  = gSystem->GetFromPipe("sw_vers -productVersion");
  //       sp += " MacOS X";
  //       printf("*  SYS: %s\n", sp.Data());
  //    }
  // } else {
  //    const char *os = gSystem->Getenv("OS");
  //    printf("*  SYS: %s %s\n", os, gSystem->Getenv("PROCESSOR_IDENTIFIER"));
  // }
  //
  // // calculate biomarks
  // printf("******************************************************************\n");
  // gBenchmark->Print("biotest");
  // float cp_mbp15 = 51.46;
  // float ct = gBenchmark->GetCpuTime("biotest");
  // float biomarks = 1000*cp_mbp15/ct;
  // printf("******************************************************************\n");
  // printf("*  BIOMARKS = %6.1f   *  biodynamo %-8s  %s/%s\n", biomarks,
  //        bdm::Version::Release(), bdm::Version::ReleaseDate(), bdm::Version::ReleaseTime());
  // printf("******************************************************************\n");
  //
  // delete gBenchmark;

  return ret;
}
