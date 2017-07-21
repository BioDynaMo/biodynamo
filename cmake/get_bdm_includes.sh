#!/bin/bash
# Use compiler -M and -MM option to extract includes from a list of source files
# cmake IMPLICIT_DEPENDS attribute only works for Makefile generators, but not
# for e.g. ninja. Therefore, this script obtains a list of dependencies
# from the compiler, parses its output. This script is called from cmake
#
# Arguments:
# $1    - Compiler binary
# $2    - List of include parameters for the compiler ("-Idir1 -Idir2 ...")
# $3    - Grep regular expression to extract only biodynamo headers from the
#         compiler output  - e.g. "^src/|^test/|^demo/"
# $ARGN - List of source files

COMPILER=$1
shift
INCLUDE_PARAM=$1
shift
BDM_HEADER_GREP_PATTERN=$1
shift

# ${COMPILER} ... |
#   get list of includes for the given sources
# sed 's/\\$//'
#   remove trailing '\' if there is one
# sed 's/\s/\n/g'
#   if there is more than one header on a line, split them
# sort | uniq
#   remove duplicates
# grep -E -v "\.o\:|\.cc|\.cxx|\.c"
#   remove lines with source and object files
# grep -E ${BDM_HEADER_GREP_PATTERN}
#   filter out biodynamo headers
# tr '\n' ';'
#   merge lines and concatenate them with ';'. Hence, cmake will treat it as a
#   list
${COMPILER} -M -MM ${INCLUDE_PARAM} -std=c++14 $@  | \
 sed 's/\\$//' | sed 's/\s/\n/g' | sort | uniq | \
 grep -E -v "\.o\:|\.cc|\.cxx|\.c" | grep -E ${BDM_HEADER_GREP_PATTERN} | tr '\n' ';'
