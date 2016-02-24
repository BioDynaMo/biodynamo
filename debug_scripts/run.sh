#!/bin/bash
# this bash script is usded to generate debugging output for the
# Java and C++ implementation of a certain class.
# Furthermore it paginates the output files and stores the page with the first
# diff in "cpp_page" and "java_page". The whole ouput can be found in "cpp" and
# "java".
#
# ./debug_scripts/run.sh CLASS_NAME
# PARAMS:
#   CLASS_NAME capitalized class name
# usage example:
# ./debug_scripts/run.sh SPACENODE

MODULE_FILE=src/main/cpp/swig/spatial_organization.i
cat $MODULE_FILE | sed "s/\/\/%native(${1})/%native(${1})/g" >/tmp/tmp123
cat /tmp/tmp123 >$MODULE_FILE
mvn -Dtest=IntracellularDiffusionTest test | grep DBG >cpp

cat $MODULE_FILE | sed "s/%native(${1})/\/\/%native(${1})/g" >/tmp/tmp123
cat /tmp/tmp123 >$MODULE_FILE
mvn -Dtest=IntracellularDiffusionTest test | grep DBG >java

cd debug_scripts/ ; ./find_first_diff.sh ../java ../cpp 0 5000 ; cd ..
