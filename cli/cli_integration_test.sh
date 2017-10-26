#!/bin/bash

echo "Starting BioDynaMo CLI Test"

cd /tmp
biodynamo new test-sim
cd test-sim
biodynamo build
biodynamo run >actual 2>&1

echo "Warning in <InitializeBioDynamo>: Config file bdm.toml not found." > expected
echo "Warning: No backup file name given. No backups will be made!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected

diff expected actual
exit $?
