#!/bin/bash

source ~/biodynamo/build/bin/thisbdm.sh
./epidemiology --inline-config '{ "bdm::OptimizationParam" : { "algorithm_" : "ParticleSwarm" } }'
