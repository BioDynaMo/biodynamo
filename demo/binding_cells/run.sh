#!/bin/bash

source ~/biodynamo/build/bin/thisbdm.sh
/home/ahmad/biodynamo/build/demo/binding_cells/build/binding_cells --data ~/csv-test/merged.csv --inline-config '{ "bdm::OptimizationParam" : { "algorithm_" : "ParticleSwarm" } }'

