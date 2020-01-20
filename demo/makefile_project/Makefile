# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

# To avoid the repitition of the --cxx-compiler and --build-type arguments, store it in
# a variable. These two options can be omitted: bdm-config will then use the default
# compiler and the build type Release.
#BUILD_TYPE=--opt
#BUILD_TYPE=--debug
BUILD_TYPE=--opt --debug
BDM_CONFIG=bdm-config $(BUILD_TYPE)
CXXFLAGS=`$(BDM_CONFIG) --cxxflags`
LINKFLAGS=`$(BDM_CONFIG) --ldflags --libs`

HEADER=src/makefile_project.h

all: makefile_project

# BioDynaMo generate code: pass all headers that contain BioDynaMo code to the
# code generator
bdm_generated_code.cc: $(HEADER)
	bdm-code-generation $@ "$(BDM_CONFIG)" -h $?

# Compile all source files
bdm_generated_code.o: bdm_generated_code.cc
	$(CXX) $(CXXFLAGS) -c $?
makefile_project.o: src/makefile_project.cc
	$(CXX) $(CXXFLAGS) -c $?

# Generate binary (link object files and libraries)
makefile_project: bdm_generated_code.o makefile_project.o
	$(CXX) $? -o $@ $(LINKFLAGS)

clean:
	rm -rf *o bdm_generated_code.cc *_rdict.pcm makefile_project
