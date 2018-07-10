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

BUILD_TYPE=Release
# To avoid the repitition of the --cxx-compiler and --build-type arguments, store it in
# a variable. These two options can be omitted: bdm-config will then use the default
# compiler and the build type Release.
BDM_CONFIG=bdm-config --cxx-compiler=$(CXX) --build-type=$(BUILD_TYPE)
CXXFLAGS=`$(BDM_CONFIG) --cxxall`
LINKFLAGS=`$(BDM_CONFIG) --linkflags`

HEADER=my-simulation.h

all: my-simulation

# BioDynaMo generate code: pass all headers that contain BioDynaMo code to the
# code generator
bdm_generated_code.cc: $(HEADER)
	bdm-code-generation $@ "$(BDM_CONFIG)" -h $?

# Compile all source files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $?

# Generate binary (link object files and libraries)
my-simulation: bdm_generated_code.o my-simulation.o
	$(CXX) $? -o $@ $(LINKFLAGS)

clean:
	@# Delete the bdm-config cache
	bdm-config --clean
	rm -rf *o bdm_generated_code.cc *_rdict.pcm my-simulation
