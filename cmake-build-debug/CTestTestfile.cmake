# CMake generated Testfile for 
# Source directory: /home/uriel/Github/biodynamo
# Build directory: /home/uriel/Github/biodynamo/cmake-build-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(runBiodynamoTestsMain "/home/uriel/Github/biodynamo/cmake-build-debug/runBiodynamoTestsMain")
add_test(valgrind_runBiodynamoTestsMain "/home/uriel/Github/biodynamo/util/valgrind.sh" "./runBiodynamoTestsMain" "--" "--gtest_filter=-*DeathTest.*:IOTest.InvalidRead:SchedulerTest.Backup:ResourceManagerTest.SortAndApplyOnAllElementsParallel*:InlineVector*")
