#!/bin/bash
# this bash script is used to generate debugging output for the
# Java and C++ implementation of a certain class.
# Furthermore it paginates the output files and stores the page with the first
# diff in "cpp_page" and "java_page". The whole ouput can be found in "cpp" and
# "java".
#
# ./debug_scripts/run.sh

CLASS=PHYSICALNODE
TEST=SomaClusteringTest

replace() {
  #$1 file
  #$2 old string
  #$3 new string
  cat $1 | sed "s|${2}|${3}|g" >/tmp/tmp123
  cat /tmp/tmp123 >$1
}

CONFIG_FILE=src/main/cpp/swig/config.i

# execute native code
cat $CONFIG_FILE | sed "s/\/\/%native(${CLASS})/%native(${CLASS})/g" >/tmp/tmp123
cat /tmp/tmp123 >$CONFIG_FILE
mvn clean -Dtest=${TEST} test | grep DBG >cpp


# execute java code
cat $CONFIG_FILE | sed "s/%native(${CLASS})/\/\/%native(${CLASS})/g" >/tmp/tmp123
cat /tmp/tmp123 >$CONFIG_FILE
mvn clean -Dtest=${TEST} test | grep DBG >java

cd debug_scripts/ ; ./find_first_diff.sh ../java ../cpp 0 20000 ; cd ..
