#!/bin/bash

mpirun -n 2 -use-hwthread-cpus build/binding_cells --config=params.json
