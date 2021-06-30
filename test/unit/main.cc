// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include <iostream>
#include "core/util/string.h"
#include "gtest/gtest.h"

void HandleFlakyTests(int& failed_cnt, std::stringstream& filter) {
  auto unit_test = ::testing::UnitTest::GetInstance();
  for (int i = 0; i < unit_test->total_test_case_count(); ++i) {
    const auto& test_case = *unit_test->GetTestCase(i);
    for (int j = 0; j < test_case.total_test_count(); ++j) {
      const auto& test_info = *test_case.GetTestInfo(j);
      // process failed flaky test
      if (test_info.result()->Failed() &&
          bdm::StartsWith(test_case.name(), "FLAKY_")) {
        failed_cnt--;
        filter << test_case.name() << "." << test_info.name() << ":";
      }
    }
  }
}

int RunAllTests() {
  auto all_passed = RUN_ALL_TESTS();
  if (all_passed == 0) {
    return 0;
  }
  return ::testing::UnitTest::GetInstance()->failed_test_case_count();
}

int main(int argc, char** argv) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ::testing::InitGoogleTest(&argc, argv);
  auto failed_cnt = RunAllTests();

  int repeat = 2;
  // Repeat failing flaky tests up to `repeat` times
  while (repeat-- > 0 && failed_cnt != 0) {
    std::stringstream filter;
    HandleFlakyTests(failed_cnt, filter);
    ::testing::GTEST_FLAG(filter) = filter.str().c_str();
    if (filter.str() == "") {
      break;
    }
    std::cout << "Rerunning the following failed flaky test(s):" << std::endl;
    auto failed_flaky_cnt = RunAllTests();
    if (failed_flaky_cnt == 0) {
      break;
    }
    failed_cnt += failed_flaky_cnt;
  }
  return failed_cnt;
}
